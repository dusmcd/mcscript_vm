#include <chunk.h>
#include <stdio.h>
#include <common.h>
#include <debug.h>


int main() {
  Chunk chunk;
  initChunk(&chunk);
  int constant = addConstant(&chunk, 1.5);

  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);

  constant = addConstant(&chunk, 1000);

  writeChunk(&chunk, OP_CONSTANT, 124);
  writeChunk(&chunk, constant, 124);
  writeChunk(&chunk, OP_RETURN, 124);

  disassembleChunk(&chunk, "instructions");

  freeChunk(&chunk);

  return 0;
}
