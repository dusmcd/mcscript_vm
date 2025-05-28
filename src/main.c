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

  int constant = addConstant(&chunk, 1.5);

  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);

  constant = addConstant(&chunk, 2.0);

  writeChunk(&chunk, OP_CONSTANT, 124);
  writeChunk(&chunk, constant, 124);
  writeChunk(&chunk, OP_ADD, 124);

  constant = addConstant(&chunk, 3);

  writeChunk(&chunk, OP_CONSTANT, 124);
  writeChunk(&chunk, constant, 124);

  writeChunk(&chunk, OP_DIVIDE, 124);
  writeChunk(&chunk, OP_RETURN, 124);

  disassembleChunk(&chunk, "instructions");
  printf("\n");
  if (interpret(vm) == RUNTIME_ERROR) {
    printf("runtime error\n");
    return 1;
  }
  

  freeChunk(&chunk);
  freeVM(vm);
  vm = NULL;

  return 0;
}
