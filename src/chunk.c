#include <value.h>
#include <chunk.h>
#include <memory.h>
#include <stdlib.h>


void initChunk(Chunk* chunk) {
  chunk->code = NULL;
  chunk->capacity = 0;
  chunk->count = 0;

  chunk->lines = NULL;
  initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);

    // line information
    chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
  }

  chunk->lines[chunk->count] = line;

  chunk->code[chunk->count] = byte;
  chunk->count++;
}


int addConstant(Chunk* chunk, Value val) {
  writeValueArray(&chunk->constants, val);
  return chunk->constants.count - 1;
}

void freeChunk(Chunk* chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  FREE_ARRAY(int, chunk->lines, chunk->capacity);
  initChunk(chunk);
  freeValueArray(&chunk->constants);
}
