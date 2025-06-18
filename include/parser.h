#ifndef MCSCRIPT_VM_PARSER_H
#define MCSCRIPT_VM_PARSER_H 

#include <scanner.h>
#include <ast.h>

/**
 * precedence hierarchy for different operators
 */
typedef enum {
  PREC_NONE,
  PREC_ASSIGN,
  PREC_EQUALITY,
  PREC_COMPARISON,
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

/**
 * functions for parsing prefix (i.e., unary) expressions
 */
typedef Expression(*PrefixFn)(Parser*, Scanner*);

/**
 * functions for parsing infix (i.e., binary) expressions
 */
typedef Expression(*InfixFn)(Parser*, Scanner*, Expression);


typedef struct {
  PrefixFn prefix;
  InfixFn infix;
  Precedence precedence;
} ParserRule;

Statements parse(Parser* parser, const char* source);
void freeStatements(Statements* statements);
void freePrefix(Prefix* prefix);
void freeInfix(Infix* infix);
void freeGrouped(Group* group);
void freeExpression(Expression* expr);

#endif
