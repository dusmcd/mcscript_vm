#include <debug.h>
#include <stdio.h>
#include <stdint.h>
#include <common.h>

void disassembleChunk(Chunk* chunk, const char* name) {
  printf("== %s ==\n", name);

  for (size_t offset = 0; offset < chunk->count;) {
    offset = disassembleInstruction(chunk, offset);
  }
}

static int simpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

int disassembleInstruction(Chunk* chunk, int offset) {
  printf("%04d ", offset);
  uint8_t instruction = chunk->code[offset];

  switch(instruction) {
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    default:
      printf("%d Unknown operator\n", instruction);
      return offset + 1;

  }
}
