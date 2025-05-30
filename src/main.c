#include <chunk.h>
#include <stdio.h>
#include <common.h>
#include <debug.h>
#include <vm.h>


int main() {
  Chunk chunk;
  initChunk(&chunk);

  VM* vm = initVM(&chunk);
  if (vm == NULL) {
    printf("insufficient memory. Terminating program\n");
    return -1;
  }

  freeChunk(&chunk);
  freeVM(vm);
  vm = NULL;

  return 0;
}
