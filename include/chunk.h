#ifndef MCSCRIPT_VM_CHUNK_H
#define MCSCRIPT_VM_CHUNK_H

#include <stdint.h>
#include <value.h>

typedef struct {
  int capacity;
  int count;
  uint8_t* code;
  ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte);
int addConstant(Chunk* chunk, Value val);
void freeChunk(Chunk* chunk);

#endif
