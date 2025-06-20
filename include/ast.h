#ifndef MCSCRIPT_VM_AST_H
#define MCSCRIPT_VM_AST_H

#include <scanner.h>
#include <stdbool.h>

typedef struct expression Expression;

/**
 * header file for data structures related to abstract syntax tree (i.e., "ast")
 */

typedef enum {
  STMT_RETURN,
  STMT_VAR,
  STMT_EXPR,
  STMT_NULL
} StatementType;

typedef enum {
  EXPR_NUMBER,
  EXPR_PREFIX,
  EXPR_INFIX,
  EXPR_GROUP,
  EXPR_BOOL,
  EXPR_STRING,
  EXPR_ERROR,
  EXPR_NULL
} ExpressionType;

typedef struct {
  double value;
  Token token;
} Number;

typedef struct {
  Token token;
  bool value;
} Boolean;

/**
 * e.g., -6, !10
 */
typedef struct {
  Token token;
  TokenType operator;
  Expression* expression;
} Prefix;

/**
 * e.g., 5 + 10, 6 * 2, etc.
 */
typedef struct {
  Token token;
  Expression* left;
  TokenType operator;
  Expression* right;
} Infix;

typedef struct {
  Token token;
  Expression* expr;
} Group;

typedef struct {
  Token token;
} String;


/**
 * all the structures for different expression types
 */
typedef union {
  Number number; // => EXPR_NUMBER
  Prefix prefix; // => EXPR_PREFIX
  Infix infix; // => EXPR_INFIX
  Group group; // => EXPR_GROUP
  Boolean boolean; // => EXPR_BOOL
  String string; // => EXPR_STRING
} ExpressionData;

/**
 * a generic container for all expressions
 */
struct expression{
  ExpressionType type;
  ExpressionData data;
};

typedef struct {
  Token token;
  Expression expression;
} ReturnStatement;

typedef struct {
  int length;
  const char* start;
} Identifier;

typedef struct {
  Token token;
  Identifier name;
  Expression value;
} VarStatement;

typedef struct {
  Token token;
  Expression expression;
} ExpressionStatement;

/**
 * all the structures for different statement types
 */
typedef union {
  ReturnStatement returnStmt; // => STMT_RETURN
  VarStatement varStmt; // => STMT_VAR
  ExpressionStatement expressionStmt; // => STMT_EXPR
} StatementData;

/**
 * generic container for all statements
 */
typedef struct {
  StatementType type;
  StatementData data;
} Statement;

/**
 * dynamic array of statements
 */
typedef struct {
  int count;
  int capacity;
  Statement* stmts;
} Statements;

#endif
