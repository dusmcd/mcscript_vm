#ifndef MCSCRIPT_VM_CHUNK_H
#define MCSCRIPT_VM_CHUNK_H

#include <stdint.h>

typedef struct {
  int capacity;
  int count;
  uint8_t* code;
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte);
void freeChunk(Chunk* chunk);

#endif
