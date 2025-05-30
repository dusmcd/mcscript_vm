#include <compiler.h>
#include <scanner.h>
#include <chunk.h>
#include <stdlib.h>

Chunk* compile(const char* source) {
  Scanner* scanner = initScanner(source);

  Token token = scanToken(scanner);
  while (token.type != TOKEN_EOF) {
    token = scanToken(scanner);
  }


  freeScanner(scanner);
  Chunk* chunk = (Chunk*)malloc(sizeof(Chunk));
  return chunk;
}

