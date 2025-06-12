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
    writeChunk(chunk, OP_NEGATE, prefix->token.line);
    compileExpression(chunk, prefix->expression);
  } else {
    // implement bang operator later
  }
}

static bool compileExpression(Chunk* chunk, Expression* expr) {
  switch(expr->type) {
    case EXPR_PREFIX: {
      Prefix prefix = expr->data.prefix;
    case EXPR_NUMBER: {
      Number number = expr->data.number;
      int i = addConstant(chunk, number.value); 
      writeChunk(chunk, OP_CONSTANT, number.token.line);
      return true;
    }
    case EXPR_ERROR:
      return false;
    }
  }
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
      return compileExpression(chunk, &expr);
    }
    case STMT_NULL:
      return true;
  } 
  return true;
}

bool compile(Chunk* chunk, const Statements* statements) {
  for (int i = 0; i < statements->count; i++) {
    Statement stmt = statements->stmts[i];
    bool isError = compileStatement(chunk, &stmt);
    if (isError) return false;
  }
  return true;
}

