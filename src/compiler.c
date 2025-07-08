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

static bool compileExpression(VM*, Expression*);

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

static bool compileInfix(VM* vm, Infix* infix) {
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

static bool compileExpression(VM* vm, Expression* expr) {
  switch(expr->type) {
    case EXPR_PREFIX: {
      Prefix prefix = expr->data.prefix;
      if (compilePrefix(vm, &prefix)) break;
      return false;
    case EXPR_INFIX: {
      Infix infix = expr->data.infix;
      if (compileInfix(vm, &infix)) break;
      return false;
    }
    case EXPR_GROUP: {
      Group group = expr->data.group;
      if (compileExpression(vm, group.expr)) break;
      return false;
    }
    case EXPR_NUMBER: {
      Number number = expr->data.number;
      Value val = NUMBER_VAL(number.value);
      writeConstant(vm->chunk, val, number.token.line);
      break;
    }
    case EXPR_STRING: {
      const char* str = createString(expr);
      Obj* obj = (Obj*)allocateString(vm, str);
      if (obj == NULL) {
        return false;
      }

      free((char*)str);
      str = NULL;
      Value val = OBJ_VAL(obj);
      writeConstant(vm->chunk, val, expr->data.string.token.line);
      break;
    }
    case EXPR_IDENT: {
      Identifier ident = expr->data.identifier;
      char* name = createName(&ident);
      Obj* obj = (Obj*)allocateString(vm, name);
      if (obj == NULL) {
        free(name);
        return false;
      }

      free(name);
      name = NULL;
      Value val = OBJ_VAL(obj);
      writeConstant(vm->chunk, val, ident.token.line);

      writeChunk(vm->chunk, OP_GET_GLOBAL, ident.token.line);
      break;
    }
    case EXPR_ERROR:
      return false;
    }
    case EXPR_BOOL:
      if (expr->data.boolean.value) {
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

static bool compileVarStatement(VM* vm, const Statement* stmt) {
  Identifier ident = stmt->data.varStmt.name;
  char* name = createName(&ident);

  Obj* obj = (Obj*)allocateString(vm, name);
  free(name);
  name = NULL;
  Value val = OBJ_VAL(obj);
  writeConstant(vm->chunk, val, stmt->data.varStmt.token.line);

  Expression expr = stmt->data.varStmt.value;
  if (!compileExpression(vm, &expr)) {
    return false;
  }


  writeChunk(vm->chunk, OP_DEFINE_GLOBAL, stmt->data.varStmt.token.line);
  return true;
}

static bool compileStatement(VM* vm, const Statement* stmt) {
  switch(stmt->type) {
    case STMT_RETURN:
      // compile return statement
      break;
    case STMT_VAR:
      return compileVarStatement(vm, stmt);
    case STMT_EXPR: {
      Expression expr = stmt->data.expressionStmt.expression;
      bool result = compileExpression(vm, &expr);
      freeExpression(&expr);
      return result;
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

