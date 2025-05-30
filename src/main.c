#include <chunk.h>
#include <stdio.h>
#include <common.h>
#include <debug.h>
#include <vm.h>


int main() {
  Chunk chunk;
  initChunk(&chunk);

  VM vm;
  initVM(&vm, &chunk);

  const char* source = "var x = 10;";
  interpret(&vm, source);
  
  freeChunk(&chunk);
  freeVM(&vm);

  return 0;
}
