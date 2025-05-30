#include <compiler.h>
#include <scanner.h>
#include <chunk.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

static char* getTokenLiteral(Token token) {
  char* literal = (char*)malloc(token.length + 1);
  for (int i = 0; i < token.length; i++) {
    literal[i] = token.start[i];
  }

  literal[token.length] = '\0';

  return literal;
}

void compile(Chunk* chunk, const char* source) {
  Scanner scanner;
  initScanner(&scanner, source);

  while (true) {
    Token token = scanToken(&scanner);
    if (token.type == TOKEN_EOF) {
      break;
    }

    char* literal = getTokenLiteral(token);
    printf("{ type: %d, literal: %s }\n", token.type, literal);
    free(literal);
  }


  freeScanner(&scanner);
}

