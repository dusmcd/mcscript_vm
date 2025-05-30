#include <compiler.h>
#include <scanner.h>
#include <chunk.h>
#include <stdlib.h>

void compile(Chunk* chunk, const char* source) {
  Scanner scanner;
  initScanner(&scanner, source);

  Token token = scanToken(&scanner);
  while (token.type != TOKEN_EOF) {
    token = scanToken(&scanner);
  }


  freeScanner(&scanner);
}

