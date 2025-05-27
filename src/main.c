#include <chunk.h>
#include <stdio.h>
#include <common.h>
#include <debug.h>


int main() {
  Chunk chunk;
  initChunk(&chunk);
  writeChunk(&chunk, OP_RETURN);
  writeChunk(&chunk, 15);

  disassembleChunk(&chunk, "instructions");

  freeChunk(&chunk);

  return 0;
}
