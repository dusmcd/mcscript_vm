#include <value.h>
#include <chunk.h>
#include <memory.h>
#include <stdlib.h>


void initChunk(Chunk* chunk) {
  chunk->code = NULL;
  chunk->capacity = 0;
  chunk->count = 0;
  initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte) {
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
  }

  chunk->code[chunk->count] = byte;
  chunk->count++;
}


int addConstant(Chunk* chunk, Value val) {
  writeValueArray(&chunk->constants, val);
  return chunk->constants.count - 1;
}

void freeChunk(Chunk* chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  initChunk(chunk);
  freeValueArray(&chunk->constants);
}
