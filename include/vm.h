#ifndef MCSCRIPT_VM_VM_H
#define MCSCRIPT_VM_VM_H

#include <stdint.h>
#include <chunk.h>
#include <value.h>
#include <table.h>

#define STACK_MAX 256
#define CURRENT_CHUNK(vm) vm->compiler->func->chunk

typedef struct Compiler Compiler;

/**
 * Holds all the data needed to execute bytecode instructions
 */
typedef struct {
  uint8_t* ip;

  /**
   * stack to keep track of values used/needed
   */
  Value valueStack[STACK_MAX];

  /**
   * points one past the last-used slot in stack
   */
  Value* stackTop;
  Obj* objects;
  Table globals;
  Compiler* compiler;
} VM;

typedef enum {
  INTERPRET_OK,
  RUNTIME_ERROR,
  COMPILE_ERROR
} InterpretResult;

void initVM(VM* vm, Compiler* compiler);
void resetVM(VM* vm);
void freeVM(VM* vm);
InterpretResult interpret(VM* vm, const char* source);
void push(VM* vm, Value val);
Value pop(VM* vm);


#endif
