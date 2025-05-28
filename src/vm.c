#include <chunk.h>
#include <common.h>
#include <vm.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <debug.h>

VM* initVM(Chunk* chunk) {
  VM* vm = (VM*)malloc(sizeof(VM));
  if (vm == NULL) {
    return NULL;
  }

  vm->chunk = chunk;
  vm->stackTop = vm->valueStack;
  return vm;
}

void freeVM(VM* vm) {
  free(vm);
}

void push(VM* vm, Value val) {
  *(vm->stackTop) = val;
  vm->stackTop++;
}

Value pop(VM* vm) {
  vm->stackTop--;
  return (*vm->stackTop);
}



static InterpretResult run(VM* vm) {

#define READ_BYTE() *(vm->ip++)
#define READ_CONSTANT() \
  (vm->chunk->constants.data[READ_BYTE()])
#define BINARY_OP(op) \
  do { \
    double b = pop(vm); \
    double a = pop(vm); \
    push(vm, a op b); \
  } while(false)

  while(true) {
#ifdef DEBUG_STACK_TRACE
  printf("[ ");
  for (Value* val = vm->valueStack; val < vm->stackTop; val++) {
    printValue(*val);
    printf(" ");
  }
  printf("]\n");
  disassembleInstruction(vm->chunk, (int)(vm->ip - vm->chunk->code));
#endif

    switch(READ_BYTE()) {
      case OP_CONSTANT: {
        Value val = READ_CONSTANT();
        push(vm, val);
        break;
      }
      case OP_ADD: {
        BINARY_OP(+);
        break;
      }
      case OP_SUBTRACT: {
        BINARY_OP(-);
        break;
      }
      case OP_MULTIPLY: {
        BINARY_OP(*);
        break;
      }
      case OP_DIVIDE: {
        BINARY_OP(/);
        break;
      }
      case OP_NEGATE: {
        push(vm, -pop(vm));
        break;
      }
      case OP_RETURN:
        printValue(pop(vm));
        printf("\n");
        return INTERPRET_OK;
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}


InterpretResult interpret(VM* vm) {
  // setting instruction pointer to first instruction in chunk
  vm->ip = vm->chunk->code;
  return run(vm);
}
