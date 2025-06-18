#include "ast.h"
#include <compiler.h>
#include <scanner.h>
#include <chunk.h>
#include <parser.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

static bool compileExpression(Chunk*, Expression*);

static void compilePrefix(Chunk* chunk, Prefix* prefix) {
  if (prefix->operator == TOKEN_MINUS) {
    compileExpression(chunk, prefix->expression);
    writeChunk(chunk, OP_NEGATE, prefix->token.line);
  } else if (prefix->operator == TOKEN_BANG){
    compileExpression(chunk, prefix->expression);
    writeChunk(chunk, OP_NOT, prefix->token.line);
  } else {
    // handle error
  }
}

static void compileInfix(Chunk* chunk, Infix* infix) {
  compileExpression(chunk, infix->left);
  compileExpression(chunk, infix->right);

  switch(infix->operator) {
    case TOKEN_PLUS: {
      writeChunk(chunk, OP_ADD, infix->token.line);
      break;
    }
    case TOKEN_MINUS:
      writeChunk(chunk, OP_SUBTRACT, infix->token.line);
      break;
    case TOKEN_STAR:
      writeChunk(chunk, OP_MULTIPLY, infix->token.line);
      break;
    case TOKEN_SLASH:
      writeChunk(chunk, OP_DIVIDE, infix->token.line);
      break;
    case TOKEN_LESS:
      writeChunk(chunk, OP_LESS, infix->token.line);
      break;
    case TOKEN_GREATER:
      writeChunk(chunk, OP_GREATER, infix->token.line);
      break;
    case TOKEN_EQUAL_EQUAL:
      writeChunk(chunk, OP_EQUAL, infix->token.line);
      break;
    case TOKEN_BANG_EQUAL:
      writeChunk(chunk, OP_EQUAL, infix->token.line);
      writeChunk(chunk, OP_NOT, infix->token.line);
      break;
    case TOKEN_LESS_EQUAL:
      writeChunk(chunk, OP_GREATER, infix->token.line);
      writeChunk(chunk, OP_NOT, infix->token.line);
      break;
    case TOKEN_GREATER_EQUAL:
      writeChunk(chunk, OP_LESS, infix->token.line);
      writeChunk(chunk, OP_NOT, infix->token.line);
      break;
    default: {
      // handle error
    }
  }
}

static bool compileExpression(Chunk* chunk, Expression* expr) {
  switch(expr->type) {
    case EXPR_PREFIX: {
      Prefix prefix = expr->data.prefix;
      compilePrefix(chunk, &prefix);
      break;
    case EXPR_INFIX: {
      Infix infix = expr->data.infix;
      compileInfix(chunk, &infix);
      break;
    }
    case EXPR_GROUP: {
      Group group = expr->data.group;
      compileExpression(chunk, group.expr);
      break;
    }
    case EXPR_NUMBER: {
      Number number = expr->data.number;
      Value val = NUMBER_VAL(number.value);
      int i = addConstant(chunk, val); 
      writeChunk(chunk, OP_CONSTANT, number.token.line);
      writeChunk(chunk, i, number.token.line);
      break;
    }
    case EXPR_ERROR:
      return false;
    }
    case EXPR_BOOL:
      if (expr->data.boolean.value) {
        writeChunk(chunk, OP_TRUE, expr->data.boolean.token.line);
      } else {
        writeChunk(chunk, OP_FALSE, expr->data.boolean.token.line);
      }
      break;
    case EXPR_NULL: 
      writeChunk(chunk, OP_NULL, expr->data.number.token.line);
      break;
  }

  return true;
}

static bool compileStatement(Chunk* chunk, const Statement* stmt) {
  switch(stmt->type) {
    case STMT_RETURN:
      // compile return statement
      break;
    case STMT_VAR:
      // compile var statement
      break;
    case STMT_EXPR: {
      Expression expr = stmt->data.expressionStmt.expression;
      bool result = compileExpression(chunk, &expr);
      freeExpression(&expr);
      return result;
    }
    case STMT_NULL:
      return true;
  } 
  return true;
}

bool compile(Chunk* chunk, const Statements* statements) {
  for (int i = 0; i < statements->count; i++) {
    Statement stmt = statements->stmts[i];
    bool isError = !compileStatement(chunk, &stmt);
    if (isError) return false;
  }
  return true;
}

