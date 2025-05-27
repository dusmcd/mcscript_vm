#include <chunk.h>
#include <stdio.h>
#include <common.h>
#include <debug.h>


int main() {
  Chunk chunk;
  initChunk(&chunk);
  int constant = addConstant(&chunk, 1.5);

  writeChunk(&chunk, OP_CONSTANT);
  writeChunk(&chunk, constant);

  constant = addConstant(&chunk, 1000);

  writeChunk(&chunk, OP_CONSTANT);
  writeChunk(&chunk, constant);
  writeChunk(&chunk, OP_RETURN);

  disassembleChunk(&chunk, "instructions");

  freeChunk(&chunk);

  return 0;
}
