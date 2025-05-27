#include <chunk.h>
#include <stdio.h>
#include <common.h>


int main() {
  Chunk chunk;
  initChunk(&chunk);
  writeChunk(&chunk, OP_RETURN);
  freeChunk(&chunk);

  printf("We made it this far\n");
  return 0;
}
