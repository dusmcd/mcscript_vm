#ifndef MCSCRIPT_VM_PARSER_H
#define MCSCRIPT_VM_PARSER_H 

#include <scanner.h>

/**
 * precedence hierarchy for different operators
 */
typedef enum {
  PREC_NONE,
  PREC_ASSIGN,
  PREC_TERM,
  PREC_FACTOR,
  PREC_UNARY,
  PREC_CALL
} Precedence;



/**
 * holds the state of the tokens being analyzed
 */
typedef struct {
  Token previous;
  Token current;
} Parser;

typedef enum {
  STMT_RETURN,
  STMT_VAR,
  STMT_NULL
} StatementType;

typedef enum {
  EXPR_NUMBER,
  EXPR_NULL
} ExpressionType;

typedef struct {
  double value;
  Token token;
} Number;

/**
 * all the structures for different expression types
 */
typedef union {
  Number number; // => EXPR_NUMBER
} ExpressionData;

/**
 * a generic container for all expressions
 */
typedef struct {
  ExpressionType type;
  ExpressionData data;
} Expression;

typedef Expression(*PrefixFn)(Parser*, Scanner*);
typedef Expression(*InfixFn)(Parser*, Scanner*, Expression);

typedef struct {
  PrefixFn prefix;
  InfixFn infix;
  Precedence precedence;
} ParserRule;

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

/**
 * all the structures for different statement types
 */
typedef union {
  ReturnStatement returnStmt; // => STMT_RETURN
  VarStatement varStmt; // => STMT_VAR
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

Statements parse(Parser* parser, const char* source);
void freeStatements(Statements* statements);

#endif
