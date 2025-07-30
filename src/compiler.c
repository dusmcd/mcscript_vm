#include <memory.h>
#include <ast.h>
#include <value.h>
#include <compiler.h>
#include <scanner.h>
#include <chunk.h>
#include <parser.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <object.h>
#include <string.h>
#include <stdint.h>

static bool compileExpression(VM*, Expression*);
static bool compileStatement(VM* vm, const Statement* stmt);
static int emitJumpInstruction(VM* vm, uint8_t instr, int line);
static void patchJump(VM* vm, int offset);

static void error(const char* msg, int line) {
  fprintf(stderr, "[line %d] ERROR: %s\n", line, msg);
}

static bool compilePrefix(VM* vm, Prefix* prefix) {
  if (prefix->operator == TOKEN_MINUS) {
    compileExpression(vm, prefix->expression);
    writeChunk(vm->chunk, OP_NEGATE, prefix->token.line);
  } else if (prefix->operator == TOKEN_BANG){
    compileExpression(vm, prefix->expression);
    writeChunk(vm->chunk, OP_NOT, prefix->token.line);
  } else {
    // handle error
    int line = prefix->token.line;
    char buff[256];
    snprintf(buff, sizeof(buff), "%d operator cannot be used as prefix operator",
        prefix->token.type);
    error(buff, line);
    return false;
  }

  return true;
}

static bool compileAndExpression(VM* vm, Infix* infix) {
  if (!compileExpression(vm, infix->left)) {
    return false;
  }
  int leftOffset = emitJumpInstruction(vm, OP_JUMP_IF_FALSE, infix->token.line);

  writeChunk(vm->chunk, OP_POP, 0);
  if (!compileExpression(vm, infix->right)) {
    return false;
  }
  
  patchJump(vm, leftOffset);

  return true;
}

static bool compileOrExpression(VM* vm, Infix* infix) {
  if (!compileExpression(vm, infix->left)) {
    return false;
  }
  int leftOffset = emitJumpInstruction(vm, OP_JUMP_IF_TRUE, infix->token.line);

  writeChunk(vm->chunk, OP_POP, 0);
  if (!compileExpression(vm, infix->right)) {
    return false;
  }

  patchJump(vm, leftOffset);
  return true;
}

static bool compileInfix(VM* vm, Infix* infix) {
  if (infix->operator == TOKEN_AND) {
    return compileAndExpression(vm, infix);
  }

  if (infix->operator == TOKEN_OR) {
    return compileOrExpression(vm, infix);
  }

  if (!compileExpression(vm, infix->left)) return false;
  if (!compileExpression(vm, infix->right)) return false;

  switch(infix->operator) {
    case TOKEN_PLUS: {
      writeChunk(vm->chunk, OP_ADD, infix->token.line);
      break;
    }
    case TOKEN_MINUS:
      writeChunk(vm->chunk, OP_SUBTRACT, infix->token.line);
      break;
    case TOKEN_STAR:
      writeChunk(vm->chunk, OP_MULTIPLY, infix->token.line);
      break;
    case TOKEN_SLASH:
      writeChunk(vm->chunk, OP_DIVIDE, infix->token.line);
      break;
    case TOKEN_LESS:
      writeChunk(vm->chunk, OP_LESS, infix->token.line);
      break;
    case TOKEN_GREATER:
      writeChunk(vm->chunk, OP_GREATER, infix->token.line);
      break;
    case TOKEN_EQUAL_EQUAL:
      writeChunk(vm->chunk, OP_EQUAL, infix->token.line);
      break;
    case TOKEN_BANG_EQUAL:
      writeChunk(vm->chunk, OP_EQUAL, infix->token.line);
      writeChunk(vm->chunk, OP_NOT, infix->token.line);
      break;
    case TOKEN_LESS_EQUAL:
      writeChunk(vm->chunk, OP_GREATER, infix->token.line);
      writeChunk(vm->chunk, OP_NOT, infix->token.line);
      break;
    case TOKEN_GREATER_EQUAL:
      writeChunk(vm->chunk, OP_LESS, infix->token.line);
      writeChunk(vm->chunk, OP_NOT, infix->token.line);
      break;
    default: {
      // handle error
      char buff[256];
      snprintf(buff, sizeof(buff), "%d is not a binary operator\n",
          infix->token.type);
      error(buff, infix->token.line);
      return false;
    }
  }

  return true;
}

static void writeConstant(Chunk* chunk, Value val, int line) {
  int i = addConstant(chunk, val);
  writeChunk(chunk, OP_CONSTANT, line);
  writeChunk(chunk, i, line);
}

static char* createName(const Identifier* ident) {
  char* name = ALLOCATE(char, ident->length + 1);
  memcpy(name, ident->start, ident->length);
  name[ident->length] = '\0';

  return name;
}

static int resolveLocal(VM* vm, Identifier ident) {
  Compiler* compiler = vm->compiler;
  for (int i = compiler->localCount - 1; i >= 0; i--) {
    Local local = compiler->locals[i];
    if (local.name.length == ident.token.length) {
      const char* localStart = local.name.start;
      const char* identStart = ident.token.start;
      int length = ident.token.length;
      if (memcmp(localStart, identStart, length) == 0) {
        return i;
      } 
    }
  }
  return -1;
}

static bool compileIdentifier(VM* vm, Identifier ident, bool assign) {
  Compiler* compiler = vm->compiler;
  int arg = resolveLocal(vm, ident);
  uint8_t opCode;


  if (arg == -1) {
    char* name = createName(&ident);
    Obj* obj = (Obj*)allocateString(vm, name);
    if (obj == NULL) {
      free(name);
      return false;
    }
    Value val = OBJ_VAL(obj);
    writeConstant(vm->chunk, val, ident.token.line);
    opCode = assign ? OP_SET_GLOBAL : OP_GET_GLOBAL;
  } else {
    Value val = NUMBER_VAL((uint8_t)arg);
    writeConstant(vm->chunk, val, ident.token.line);
    opCode = assign ? OP_SET_LOCAL : OP_GET_LOCAL;
  }

  writeChunk(vm->chunk, opCode, ident.token.line);
  return true;
}

static bool compileExpression(VM* vm, Expression* expr) {
  switch(expr->type) {
    case EXPR_PREFIX: {
      Prefix prefix = AS_EXPR_PREFIX((*expr));
      if (compilePrefix(vm, &prefix)) break;
      return false;
    case EXPR_INFIX: {
      Infix infix = AS_EXPR_INFIX((*expr));
      if (compileInfix(vm, &infix)) break;
      return false;
    }
    case EXPR_GROUP: {
      Group group = AS_EXPR_GROUP((*expr));
      if (compileExpression(vm, group.expr)) break;
      return false;
    }
    case EXPR_NUMBER: {
      Number number = AS_EXPR_NUM((*expr));
      Value val = NUMBER_VAL(number.value);
      writeConstant(vm->chunk, val, number.token.line);
      break;
    }
    case EXPR_STRING: {
      char* str = createString(expr);
      Obj* obj = (Obj*)allocateString(vm, str);
      if (obj == NULL) {
        free(str);
        return false;
      }
      Value val = OBJ_VAL(obj);
      writeConstant(vm->chunk, val, expr->data.string.token.line);
      break;
    }
    case EXPR_IDENT: {
      Identifier ident = AS_EXPR_IDENT((*expr));
      if (!compileIdentifier(vm, ident, false)) {
        error("insufficient memory", ident.token.line);
        return false;
      }
      break;
    }
    case EXPR_ERROR:
      return false;
    }
    case EXPR_BOOL:
      if (AS_EXPR_BOOL((*expr)).value) {
        writeChunk(vm->chunk, OP_TRUE, expr->data.boolean.token.line);
      } else {
        writeChunk(vm->chunk, OP_FALSE, expr->data.boolean.token.line);
      }
      break;
    case EXPR_NULL: 
      writeChunk(vm->chunk, OP_NULL, expr->data.number.token.line);
      break;
  }

  return true;
}

static bool addLocal(VM* vm, Token name) {
  Compiler* compiler = vm->compiler;
  Local* local = &compiler->locals[compiler->localCount++];
  local->name = name;
  local->depth = compiler->scopeDepth;

  if (compiler->localCount == UINT8_COUNT) {
    error("Too many local variables in function", name.line);
    return false;
  }

  return true;
}

static bool compileVarStatement(VM* vm, const Statement* stmt) {
  Identifier ident = AS_VARSTMT((*stmt)).name;
  bool isLocal = vm->compiler->scopeDepth > 0;

  if (!isLocal) {
    char* name = createName(&ident);

    Obj* obj = (Obj*)allocateString(vm, name);
    Value val = OBJ_VAL(obj);
    writeConstant(vm->chunk, val, stmt->data.varStmt.token.line);
  }
  
  Expression expr = AS_VARSTMT((*stmt)).value;
  if (!compileExpression(vm, &expr)) {
    return false;
  }

  // if scope depth is greater than zero, then variable is local
  // so we do not want to create a global instruction
  if (isLocal) {
    return addLocal(vm, ident.token);
  }


  writeChunk(vm->chunk, OP_DEFINE_GLOBAL, stmt->data.varStmt.token.line);
  return true;
}

static bool compileBlockStatement(VM* vm, const Statement* stmt) {
  vm->compiler->scopeDepth++;
  Statements stmts = AS_BLOCKSTMT((*stmt)).stmts;

  for (int i = 0; i < stmts.count; i++) {
    Statement inner = stmts.stmts[i];
    if (!compileStatement(vm, &inner)) return false;
  }

  vm->compiler->scopeDepth--;
  Compiler* compiler = vm->compiler;
  while(compiler->localCount > 0 &&
      compiler->locals[compiler->localCount - 1].depth >
        compiler->scopeDepth) {
    writeChunk(vm->chunk, OP_POP, 0);
    compiler->localCount--;
  }

  return true;
}

static int emitJumpInstruction(VM* vm, uint8_t instr, int line) {
  writeChunk(vm->chunk, instr, line);

  // leave two bytes for jump offset
  writeChunk(vm->chunk, 0xff, 0);
  writeChunk(vm->chunk, 0xff, 0);

  return vm->chunk->count - 2;
}

static void patchJump(VM* vm, int offset) {
  int jump = vm->chunk->count - offset - 2;

  if (jump > UINT16_MAX) {
    error("too many bytes in jump", 0);
    return;
  }

  // breaking out jump integer into 2 different bytes
  vm->chunk->code[offset] = (jump >> 8) & 0xff;
  vm->chunk->code[offset + 1] = jump & 0xff;
}

static bool compileIfStatement(VM* vm, const Statement* stmt) {
  IfStatement is = AS_IFSTMT((*stmt));

  if (!compileExpression(vm, &is.condition)) {
    return false;
  }

  int thenOffset = emitJumpInstruction(vm, OP_JUMP_IF_FALSE, is.token.line);
  writeChunk(vm->chunk, OP_POP, is.token.line);

  Statement block = {.type = STMT_BLOCK, .data = {.blockStmt = is.block}};
  if (!compileBlockStatement(vm, &block)) {
    return false;
  }

  int elseOffset = emitJumpInstruction(vm, OP_JUMP, is.elseBlock.token.line);

  patchJump(vm, thenOffset);
  writeChunk(vm->chunk, OP_POP, is.token.line);

  if (is.elseBlock.token.type != TOKEN_NULL) {
    Statement elseBlock = {.type = STMT_BLOCK, .data = {.blockStmt = is.elseBlock}};
    if (!compileBlockStatement(vm, &elseBlock)) {
      return false;
    }
  }

  patchJump(vm, elseOffset);


  return true;
}

static void emitLoop(VM* vm, int loopStart, int line) {
  writeChunk(vm->chunk, OP_LOOP, line);
  int jump = (vm->chunk->count - loopStart) + 2;

  if (jump > UINT16_MAX) {
    error("loop body too large", line);
    return;
  }

  writeChunk(vm->chunk, (jump >> 8) & 0xff, line);
  writeChunk(vm->chunk, jump & 0xff, line);
}

static bool compileWhileStatement(VM* vm, const Statement* stmt) {
  WhileStatement ws = AS_WHILESTMT((*stmt));
  int loopStart = vm->chunk->count;
  if (!compileExpression(vm, &ws.condition)) {
    return false;
  }
  int exitOffset = emitJumpInstruction(vm, OP_JUMP_IF_FALSE, ws.token.line);

  writeChunk(vm->chunk, OP_POP, ws.token.line);
  Statement block = {.type = STMT_BLOCK, .data = {.blockStmt = ws.block}};
  if (!compileBlockStatement(vm, &block)) {
    return false;
  }
  emitLoop(vm, loopStart, ws.token.line);

  patchJump(vm, exitOffset);
  writeChunk(vm->chunk, OP_POP, ws.token.line);

  return true;
}

static bool compileAssignStatement(VM* vm, const Statement* stmt) {
  AssignStatement as = AS_ASSIGNSTMT((*stmt));
  if (!compileExpression(vm, &as.value)) {
    return false;
  }

  if (!compileIdentifier(vm, as.name, true)) {
    return false;
  }

  return true;
}

static bool compileStatement(VM* vm, const Statement* stmt) {
  switch(stmt->type) {
    case STMT_RETURN:
      // compile return statement
      break;
    case STMT_VAR:
      return compileVarStatement(vm, stmt);
    case STMT_ASSIGN:
      return compileAssignStatement(vm, stmt);
    case STMT_EXPR: {
      Expression expr = AS_EXPRSTMT((*stmt)).expression;
      bool result = compileExpression(vm, &expr);
      freeExpression(&expr);
      return result;
    }
    case STMT_BLOCK: {
      return compileBlockStatement(vm, stmt);
    }
    case STMT_IF: {
      return compileIfStatement(vm, stmt);
    }
    case STMT_WHILE: {
      return compileWhileStatement(vm, stmt);
    }
    case STMT_NULL:
      return true;
  } 
  return true;
}

bool compile(VM* vm, const Statements* statements) {
  for (int i = 0; i < statements->count; i++) {
    Statement stmt = statements->stmts[i];
    bool isError = !compileStatement(vm, &stmt);
    if (isError) return false;
  }
  return true;
}

void initCompiler(Compiler *compiler) {
  compiler->scopeDepth = 0;
  compiler->localCount = 0;
}
