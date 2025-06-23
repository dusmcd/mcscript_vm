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

static bool compileExpression(VM*, Expression*);

static void compilePrefix(VM* vm, Prefix* prefix) {
  if (prefix->operator == TOKEN_MINUS) {
    compileExpression(vm, prefix->expression);
    writeChunk(vm->chunk, OP_NEGATE, prefix->token.line);
  } else if (prefix->operator == TOKEN_BANG){
    compileExpression(vm, prefix->expression);
    writeChunk(vm->chunk, OP_NOT, prefix->token.line);
  } else {
    // handle error
  }
}

static void compileInfix(VM* vm, Infix* infix) {
  compileExpression(vm, infix->left);
  compileExpression(vm, infix->right);

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
    }
  }
}

static void writeConstant(Chunk* chunk, Value val, int line) {
  int i = addConstant(chunk, val);
  writeChunk(chunk, OP_CONSTANT, line);
  writeChunk(chunk, i, line);
}

static bool compileExpression(VM* vm, Expression* expr) {
  switch(expr->type) {
    case EXPR_PREFIX: {
      Prefix prefix = expr->data.prefix;
      compilePrefix(vm, &prefix);
      break;
    case EXPR_INFIX: {
      Infix infix = expr->data.infix;
      compileInfix(vm, &infix);
      break;
    }
    case EXPR_GROUP: {
      Group group = expr->data.group;
      compileExpression(vm, group.expr);
      break;
    }
    case EXPR_NUMBER: {
      Number number = expr->data.number;
      Value val = NUMBER_VAL(number.value);
      writeConstant(vm->chunk, val, number.token.line);
      break;
    }
    case EXPR_STRING: {
      Obj* obj = createObject(vm, expr, OBJ_STRING);
      if (obj == NULL) {
        return false;
      }
      Value val = OBJ_VAL(obj);
      writeConstant(vm->chunk, val, expr->data.string.token.line);
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

static bool compileStatement(VM* vm, const Statement* stmt) {
  switch(stmt->type) {
    case STMT_RETURN:
      // compile return statement
      break;
    case STMT_VAR:
      // compile var statement
      break;
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

