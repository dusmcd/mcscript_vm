#ifndef MCSCRIPT_VM_SCANNER_H
#define MCSCRIPT_VM_SCANNER_H

typedef struct {
  const char* start;
  const char* current;
  int line;
} Scanner;

typedef enum {
  TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
  TOKEN_EOF
  /**
   * TODO: add remaining token types
   */
} TokenType;

typedef struct {
  const char* start;
  int length;
  TokenType type;
  int line;
} Token;

Scanner* initScanner(const char* source);
Token scanToken(Scanner* scanner);
void freeScanner(Scanner* scanner);

#endif
