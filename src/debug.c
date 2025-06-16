#include "chunk.h"
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

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
  printf("%s '", name); 
  uint8_t valIndex = chunk->code[offset + 1];
  Value val = chunk->constants.data[valIndex];
  printValue(val);
  printf("'\n");

  return offset + 2;
}

int disassembleInstruction(Chunk* chunk, int offset) {
  printf("%04d ", offset);
  uint8_t instruction = chunk->code[offset];

  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
    printf("  |  ");
  } else {
    printf("  %d  ", chunk->lines[offset]);
  }

  switch(instruction) {
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    case OP_CONSTANT:
      return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_NEGATE:
      return simpleInstruction("OP_NEGATE", offset);
    case OP_ADD:
      return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:
      return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
      return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
      return simpleInstruction("OP_DIVIDE", offset);
    case OP_TRUE:
      return simpleInstruction("OP_TRUE", offset);
    case OP_FALSE:
      return simpleInstruction("OP_FALSE", offset);
    case OP_NULL:
      return simpleInstruction("OP_NULL", offset);
    default:
      printf("%d Unknown operator\n", instruction);
      return offset + 1;

  }
}
