#ifndef MCSCRIPT_VM_AST_H
#define MCSCRIPT_VM_AST_H

#include <scanner.h>
#include <stdbool.h>

#define AS_EXPR_NUM(expr) expr.data.number
#define AS_EXPR_INFIX(expr) expr.data.infix
#define AS_EXPR_PREFIX(expr) expr.data.prefix
#define AS_EXPR_GROUP(expr) expr.data.group
#define AS_EXPR_IDENT(expr) expr.data.identifier
#define AS_EXPR_BOOL(expr) expr.data.boolean

#define AS_VARSTMT(stmt) stmt.data.varStmt
#define AS_BLOCKSTMT(stmt) stmt.data.blockStmt
#define AS_EXPRSTMT(stmt) stmt.data.expressionStmt
#define AS_IFSTMT(stmt) stmt.data.ifStmt
#define AS_WHILESTMT(stmt) stmt.data.whileStmt
#define AS_ASSIGNSTMT(stmt) stmt.data.assignStmt

typedef struct expression Expression;
typedef struct Statement Statement;



/**
 * header file for data structures related to abstract syntax tree (i.e., "ast")
 */

typedef enum {
  STMT_RETURN,
  STMT_VAR,
  STMT_EXPR,
  STMT_BLOCK,
  STMT_IF,
  STMT_WHILE,
  STMT_ASSIGN,
  STMT_NULL
} StatementType;

typedef enum {
  EXPR_NUMBER,
  EXPR_PREFIX,
  EXPR_INFIX,
  EXPR_GROUP,
  EXPR_BOOL,
  EXPR_STRING,
  EXPR_IDENT,
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

typedef struct {
  int length;
  Token token;
  const char* start;
} Identifier;

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
  Identifier identifier; // => EXPR_IDENT
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
  Token token;
  Identifier name;
  Expression value;
} VarStatement;

typedef struct {
  Token token;
  Expression expression;
} ExpressionStatement;

/**
 * dynamic array of statements
 */
typedef struct {
  int count;
  int capacity;
  Statement* stmts;
} Statements;


typedef struct {
  Token token;
  Statements stmts;
} BlockStatement;

/**
 * if statement for control flow/branching
*/
typedef struct {
  Token token;
  Expression condition;
  BlockStatement block;
  BlockStatement elseBlock;
} IfStatement;

/**
 * while loop
 */
typedef struct {
  Token token;
  Expression condition;
  BlockStatement block;
} WhileStatement;

/**
 * assigning local and global variables
 * to new values
 */
typedef struct {
  Token token;
  Expression value;
  Identifier name;
} AssignStatement;

/**
 * all the structures for different statement types
 */
typedef union {
  ReturnStatement returnStmt; // => STMT_RETURN
  VarStatement varStmt; // => STMT_VAR
  ExpressionStatement expressionStmt; // => STMT_EXPR
  BlockStatement blockStmt; // => STMT_BLOCK
  IfStatement ifStmt; // => STMT_IF
  WhileStatement whileStmt; // => STMT_WHILE
  AssignStatement assignStmt; // => STMT_ASSIGN
} StatementData;

/**
 * generic container for all statements
 */
struct Statement {
  StatementType type;
  StatementData data;
};

#endif
