#include <scanner.h>
#include <stdlib.h>

void initScanner(Scanner* scanner, const char* source) {
  scanner->start = source;
  scanner->current = source;
  scanner->line = 1;
}

void freeScanner(Scanner* scanner) {
}

static char advance(Scanner* scanner) {
  scanner->current++;
  return scanner->current[-1];
}

static Token makeToken(Scanner* scanner, TokenType type) {
  Token token = {
    .type = type,
    .length = (int)(scanner->current - scanner->start),
    .line = scanner->line,
    .start = scanner->start
  };

  return token;
}


Token scanToken(Scanner* scanner) {
  scanner->start = scanner->current;
  char c = advance(scanner);
  switch(c) {
    case '(': return makeToken(scanner, TOKEN_LEFT_PAREN);
    case ')': return makeToken(scanner, TOKEN_RIGHT_PAREN);
    case '{': return makeToken(scanner, TOKEN_LEFT_BRACE);
    case '}': return makeToken(scanner, TOKEN_RIGHT_BRACE);
    case '\0': return makeToken(scanner, TOKEN_EOF);
  }

}
