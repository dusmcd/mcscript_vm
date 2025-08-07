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
static ObjFunction* endCompiler(VM* vm);
static bool compileIdentifier(VM* vm, Identifier ident, bool assign);

static void error(const char* msg, int line) {
  fprintf(stderr, "[line %d] ERROR: %s\n", line, msg);
}

static void beginScope(VM* vm) {
  vm->compiler->scopeDepth++;
}

static void endScope(VM* vm) {
  vm->compiler->scopeDepth--;
  Compiler* compiler = vm->compiler;

  while(compiler->localCount > 0 &&
      compiler->locals[compiler->localCount - 1].depth >
        compiler->scopeDepth) {
    writeChunk(&CURRENT_CHUNK(vm), OP_POP, 0);
    compiler->localCount--;
  }
}

static void compileReturnVal(VM* vm, const Expression* expr) {
  if (expr->type == EXPR_CALL) {
    Token tok = {.type = TOKEN_IDENTIFIER, .length = 6, .start = "return"};
    Identifier ident = {.length = 6, .token = tok, .start = "return"};
    compileIdentifier(vm, ident, false);
  }
}

static bool compilePrefix(VM* vm, Prefix* prefix) {
  if (prefix->operator == TOKEN_MINUS) {
    compileExpression(vm, prefix->expression);
    writeChunk(&CURRENT_CHUNK(vm), OP_NEGATE, prefix->token.line);
  } else if (prefix->operator == TOKEN_BANG){
    compileExpression(vm, prefix->expression);
    writeChunk(&CURRENT_CHUNK(vm), OP_NOT, prefix->token.line);
  } else {
    // handle error
    int line = prefix->token.line;
    char buff[256];
    snprintf(buff, sizeof(buff), "%d operator cannot be used as prefix operator",
        prefix->token.type);
    error(buff, line);
    return false;
  }

  compileReturnVal(vm, prefix->expression);

  return true;
}

static bool compileAndExpression(VM* vm, Infix* infix) {
  if (!compileExpression(vm, infix->left)) {
    return false;
  }
  compileReturnVal(vm, infix->left);
  int leftOffset = emitJumpInstruction(vm, OP_JUMP_IF_FALSE, infix->token.line);

  writeChunk(&CURRENT_CHUNK(vm), OP_POP, 0);
  if (!compileExpression(vm, infix->right)) {
    return false;
  }
  compileReturnVal(vm, infix->right);
  
  patchJump(vm, leftOffset);

  return true;
}

static bool compileOrExpression(VM* vm, Infix* infix) {
  if (!compileExpression(vm, infix->left)) {
    return false;
  }
  compileReturnVal(vm, infix->left);
  int leftOffset = emitJumpInstruction(vm, OP_JUMP_IF_TRUE, infix->token.line);

  writeChunk(&CURRENT_CHUNK(vm), OP_POP, 0);
  if (!compileExpression(vm, infix->right)) {
    return false;
  }
  compileReturnVal(vm, infix->right);


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
  compileReturnVal(vm, infix->left);

  if (!compileExpression(vm, infix->right)) return false;
  compileReturnVal(vm, infix->right);

  switch(infix->operator) {
    case TOKEN_PLUS: {
      writeChunk(&CURRENT_CHUNK(vm), OP_ADD, infix->token.line);
      break;
    }
    case TOKEN_MINUS:
      writeChunk(&CURRENT_CHUNK(vm), OP_SUBTRACT, infix->token.line);
      break;
    case TOKEN_STAR:
      writeChunk(&CURRENT_CHUNK(vm), OP_MULTIPLY, infix->token.line);
      break;
    case TOKEN_SLASH:
      writeChunk(&CURRENT_CHUNK(vm), OP_DIVIDE, infix->token.line);
      break;
    case TOKEN_LESS:
      writeChunk(&CURRENT_CHUNK(vm), OP_LESS, infix->token.line);
      break;
    case TOKEN_GREATER:
      writeChunk(&CURRENT_CHUNK(vm), OP_GREATER, infix->token.line);
      break;
    case TOKEN_EQUAL_EQUAL:
      writeChunk(&CURRENT_CHUNK(vm), OP_EQUAL, infix->token.line);
      break;
    case TOKEN_BANG_EQUAL:
      writeChunk(&CURRENT_CHUNK(vm), OP_EQUAL, infix->token.line);
      writeChunk(&CURRENT_CHUNK(vm), OP_NOT, infix->token.line);
      break;
    case TOKEN_LESS_EQUAL:
      writeChunk(&CURRENT_CHUNK(vm), OP_GREATER, infix->token.line);
      writeChunk(&CURRENT_CHUNK(vm), OP_NOT, infix->token.line);
      break;
    case TOKEN_GREATER_EQUAL:
      writeChunk(&CURRENT_CHUNK(vm), OP_LESS, infix->token.line);
      writeChunk(&CURRENT_CHUNK(vm), OP_NOT, infix->token.line);
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
        // accounting for first slot being used by compiler
        return i - 1;
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
    writeConstant(&CURRENT_CHUNK(vm), val, ident.token.line);
    opCode = assign ? OP_SET_GLOBAL : OP_GET_GLOBAL;
  } else {
    Value val = NUMBER_VAL((uint8_t)arg);
    writeConstant(&CURRENT_CHUNK(vm), val, ident.token.line);
    opCode = assign ? OP_SET_LOCAL : OP_GET_LOCAL;
  }

  writeChunk(&CURRENT_CHUNK(vm), opCode, ident.token.line);
  return true;
}

static bool compileCallExpression(VM* vm, const CallExpression* call) {

  for (int i = 0; i < call->argCount; i++) {
    if (!compileExpression(vm, call->args + i)) {
      return false;
    }
    compileReturnVal(vm, call->args + i);
    writeChunk(&CURRENT_CHUNK(vm), OP_MARK_LOCAL, call->token.line);
  }

  if (!compileIdentifier(vm, call->name, false)) {
    error("insufficient memory", call->token.line);
    return false;
  }

  writeChunk(&CURRENT_CHUNK(vm), OP_CALL, call->token.line);
  writeChunk(&CURRENT_CHUNK(vm), (uint8_t)call->argCount, call->token.line);

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
      if (compileExpression(vm, group.expr)){
        break;
      }
      return false;
    }
    case EXPR_NUMBER: {
      Number number = AS_EXPR_NUM((*expr));
      Value val = NUMBER_VAL(number.value);
      writeConstant(&CURRENT_CHUNK(vm), val, number.token.line);
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
      writeConstant(&CURRENT_CHUNK(vm), val, expr->data.string.token.line);
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
    case EXPR_CALL: {
      CallExpression call = AS_EXPR_CALL((*expr));
      if (!compileCallExpression(vm, &call)) return false;
      break;
    }
    case EXPR_ERROR:
      return false;
    }
    case EXPR_BOOL:
      if (AS_EXPR_BOOL((*expr)).value) {
        writeChunk(&CURRENT_CHUNK(vm), OP_TRUE, expr->data.boolean.token.line);
      } else {
        writeChunk(&CURRENT_CHUNK(vm), OP_FALSE, expr->data.boolean.token.line);
      }
      break;
    case EXPR_NULL: 
      writeChunk(&CURRENT_CHUNK(vm), OP_NULL, expr->data.number.token.line);
      break;
  }

  return true;
}

static bool addLocal(VM* vm, Token name, bool isArg) {
  Compiler* compiler = vm->compiler;
  Local* local = &compiler->locals[compiler->localCount++];
  local->name = name;
  local->depth = compiler->scopeDepth;

  if (compiler->localCount == UINT8_COUNT) {
    error("Too many local variables in function", name.line);
    return false;
  }

  if (!isArg) {
    writeChunk(&CURRENT_CHUNK(vm), OP_MARK_LOCAL, name.line);
  }

  return true;
}

static bool compileDeclaration(VM* vm, int line, Identifier ident, bool isArg) {
  bool isLocal = vm->compiler->scopeDepth > 0;

  if (!isLocal) {
    char* name = createName(&ident);
    Obj* obj = (Obj*)allocateString(vm, name);
    Value val = OBJ_VAL(obj);
    writeConstant(&CURRENT_CHUNK(vm), val, line);
  }
  
  // if scope depth is greater than zero, then variable is local
  // so we do not want to create a global instruction
  if (isLocal) {
    return addLocal(vm, ident.token, isArg);
  }

  writeChunk(&CURRENT_CHUNK(vm), OP_DEFINE_GLOBAL, line);
  return true;
}

static bool compileVarStatement(VM* vm, const Statement* stmt) {
  Identifier ident = AS_VARSTMT((*stmt)).name;

  Expression expr = AS_VARSTMT((*stmt)).value;
  if (!compileExpression(vm, &expr)) {
    return false;
  }
  compileReturnVal(vm, &expr);

  return compileDeclaration(vm, stmt->data.varStmt.token.line, ident, false);
}

static bool compileBlockStatement(VM* vm, const Statement* stmt) {
  Statements stmts = AS_BLOCKSTMT((*stmt)).stmts;

  for (int i = 0; i < stmts.count; i++) {
    Statement inner = stmts.stmts[i];
    if (!compileStatement(vm, &inner)) return false;
  }
  
  return true;
}

static int emitJumpInstruction(VM* vm, uint8_t instr, int line) {
  writeChunk(&CURRENT_CHUNK(vm), instr, line);

  // leave two bytes for jump offset
  writeChunk(&CURRENT_CHUNK(vm), 0xff, 0);
  writeChunk(&CURRENT_CHUNK(vm), 0xff, 0);

  return CURRENT_CHUNK(vm).count - 2;
}

static void patchJump(VM* vm, int offset) {
  int jump = CURRENT_CHUNK(vm).count - offset - 2;

  if (jump > UINT16_MAX) {
    error("too many bytes in jump", 0);
    return;
  }

  // breaking out jump integer into 2 different bytes
  CURRENT_CHUNK(vm).code[offset] = (jump >> 8) & 0xff;
  CURRENT_CHUNK(vm).code[offset + 1] = jump & 0xff;
}

static bool compileIfStatement(VM* vm, const Statement* stmt) {
  IfStatement is = AS_IFSTMT((*stmt));

  if (!compileExpression(vm, &is.condition)) {
    return false;
  }
  compileReturnVal(vm, &is.condition);

  int thenOffset = emitJumpInstruction(vm, OP_JUMP_IF_FALSE, is.token.line);
  writeChunk(&CURRENT_CHUNK(vm), OP_POP, is.token.line);

  Statement block = {.type = STMT_BLOCK, .data = {.blockStmt = is.block}};
  if (!compileStatement(vm, &block)) {
    return false;
  }

  int elseOffset = emitJumpInstruction(vm, OP_JUMP, is.elseBlock.token.line);

  patchJump(vm, thenOffset);
  writeChunk(&CURRENT_CHUNK(vm), OP_POP, is.token.line);

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
  writeChunk(&CURRENT_CHUNK(vm), OP_LOOP, line);
  int jump = (CURRENT_CHUNK(vm).count - loopStart) + 2;

  if (jump > UINT16_MAX) {
    error("loop body too large", line);
    return;
  }

  writeChunk(&CURRENT_CHUNK(vm), (jump >> 8) & 0xff, line);
  writeChunk(&CURRENT_CHUNK(vm), jump & 0xff, line);
}

static bool compileWhileStatement(VM* vm, const Statement* stmt) {
  WhileStatement ws = AS_WHILESTMT((*stmt));
  int loopStart = CURRENT_CHUNK(vm).count;
  if (!compileExpression(vm, &ws.condition)) {
    return false;
  }
  compileReturnVal(vm, &ws.condition);
  int exitOffset = emitJumpInstruction(vm, OP_JUMP_IF_FALSE, ws.token.line);

  writeChunk(&CURRENT_CHUNK(vm), OP_POP, ws.token.line);
  Statement block = {.type = STMT_BLOCK, .data = {.blockStmt = ws.block}};
  if (!compileStatement(vm, &block)) {
    return false;
  }
  emitLoop(vm, loopStart, ws.token.line);

  patchJump(vm, exitOffset);
  writeChunk(&CURRENT_CHUNK(vm), OP_POP, ws.token.line);

  return true;
}

static bool compileAssignStatement(VM* vm, const Statement* stmt) {
  AssignStatement as = AS_ASSIGNSTMT((*stmt));
  if (!compileExpression(vm, &as.value)) {
    return false;
  }
  compileReturnVal(vm, &as.value);

  if (!compileIdentifier(vm, as.name, true)) {
    return false;
  }

  return true;
}

static bool compileFunction(VM* vm, const FunctionStatement* fs) {
  Compiler compiler;
  initCompiler(vm, &compiler, TYPE_FUNCTION);
  vm->compiler->func->numArgs = fs->argCount;
  char* str = createName(&fs->name);
  vm->compiler->func->name = allocateString(vm, str);

  beginScope(vm);

  // compile args and block
  for (int i = 0; i < fs->argCount; i++) {
    if (!compileDeclaration(vm, fs->token.line, fs->args[i], true)) return false;
  }

  Statement block = {.data = {.blockStmt = fs->block}, .type = STMT_BLOCK};
  if (!compileBlockStatement(vm, &block)) return false;
  
  ObjFunction* func = endCompiler(vm);
  writeConstant(&CURRENT_CHUNK(vm), OBJ_VAL(func), fs->token.line);

  return true;
}

static bool compileFunctionStatement(VM* vm, const Statement* stmt) {
  FunctionStatement fs = AS_FUNCSTMT((*stmt));

  if (!compileFunction(vm, &fs)) {
    return false;
  }

  if (!compileDeclaration(vm, fs.token.line, fs.name, false)) {
    return false;
  }

  return true;
}

static bool compileReturnStatement(VM* vm, const Statement* stmt) {
  ReturnStatement rs = AS_RETURNSTMT((*stmt));

  if (!compileExpression(vm, &rs.expression)) {
    return false;
  }
  compileReturnVal(vm, &rs.expression);

  writeChunk(&CURRENT_CHUNK(vm), OP_RETURN, rs.token.line);
  return true;
}

static bool compileStatement(VM* vm, const Statement* stmt) {
  switch(stmt->type) {
    case STMT_RETURN:
      // compile return statement
      return compileReturnStatement(vm, stmt);
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
      beginScope(vm);
      bool result = compileBlockStatement(vm, stmt);
      endScope(vm);
      return result;
    }
    case STMT_IF: {
      return compileIfStatement(vm, stmt);
    }
    case STMT_WHILE: {
      return compileWhileStatement(vm, stmt);
    }
    case STMT_FUNCTION: {
      return compileFunctionStatement(vm, stmt);
    }
    case STMT_NULL:
      return true;
    case STMT_ERROR:
      return false;
  } 
  return true;
}

static void emitReturn(VM* vm) {
  writeChunk(&CURRENT_CHUNK(vm), OP_NULL, 0);
  writeChunk(&CURRENT_CHUNK(vm), OP_RETURN, 0);
}

static ObjFunction* endCompiler(VM* vm) {
  emitReturn(vm);

  ObjFunction* func = vm->compiler->func;
  vm->compiler = vm->compiler->enclosing;

  return func;
}

CompilerResult compile(VM* vm, const Statements* statements) {
  for (int i = 0; i < statements->count; i++) {
    Statement stmt = statements->stmts[i];
    bool isError = !compileStatement(vm, &stmt);
    if (isError) return (CompilerResult){.hasError = true};
  }
  emitReturn(vm);
  return (CompilerResult){.hasError = false, .func = endCompiler(vm)};
}

void initCompiler(VM* vm, Compiler *compiler, FunctionType type) {
  compiler->scopeDepth = 0;
  compiler->localCount = 0;

  compiler->func = NULL;
  compiler->type = type;
  compiler->enclosing = vm->compiler;
  vm->compiler = compiler;

  compiler->func = newFunction(vm);

  // reserving the first slot for the compiler
  Local* local = &compiler->locals[compiler->localCount++];
  local->name = (Token){.type = TOKEN_NULL};
  local->depth = 0;
}
