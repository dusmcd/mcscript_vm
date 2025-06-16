#ifndef MCSCRIPT_VM_SCANNER_H
#define MCSCRIPT_VM_SCANNER_H

/**
 * holds the state of lexical analysis
 * of source code
 */
typedef struct {
  /**
   * the first char of the token being analyzed
   */
  const char* start;

  /**
   * the current char of the token being analyzed
   */
  const char* current;
  int line;
} Scanner;

typedef enum {
  TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
  TOKEN_BANG, TOKEN_BANG_EQUAL,
  TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
  TOKEN_LESS, TOKEN_LESS_EQUAL,
  TOKEN_GREATER, TOKEN_GREATER_EQUAL,
  TOKEN_COMMA, TOKEN_SEMICOLON,
  TOKEN_SLASH, TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR,
  TOKEN_DOT, TOKEN_FUNCTION, TOKEN_FOR, TOKEN_WHILE,
  TOKEN_IF, TOKEN_ELSE, TOKEN_VAR, TOKEN_PRINT,
  TOKEN_TRUE, TOKEN_FALSE,
  TOKEN_IDENTIFIER, TOKEN_NUMBER, TOKEN_STRING,
  TOKEN_AND, TOKEN_OR, TOKEN_RETURN,
  TOKEN_ILLEGAL, TOKEN_EOF
} TokenType;

/**
 * a single token
 */
typedef struct {
  const char* start;
  int length;
  TokenType type;
  int line;
} Token;

void initScanner(Scanner* scanner, const char* source);
Token scanToken(Scanner* scanner);
void freeScanner(Scanner* scanner);

#endif
