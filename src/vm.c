#include <chunk.h>
#include <vm.h>
#include <stdlib.h>
#include <stdbool.h>

VM* initVM(Chunk* chunk) {
  VM* vm = (VM*)malloc(sizeof(VM));
  if (vm == NULL) {
    return NULL;
  }

  vm->chunk = chunk;
  return vm;
}

void freeVM(VM* vm) {
  free(vm);
}


static InterpretResult run(VM* vm) {
#define READ_BYTE() *(vm->ip++)
#define READ_CONSTANT() \
  (vm->chunk->constants.data[READ_BYTE()])

  while(true) {
    switch(READ_BYTE()) {
      case OP_CONSTANT: {
        Value val = READ_CONSTANT();
        break;
      }
      case OP_RETURN:
        return INTERPRET_OK;
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
}


InterpretResult interpret(VM* vm) {
  // setting instruction pointer to first instruction in chunk
  vm->ip = vm->chunk->code;
  return run(vm);
}
