#ifndef MCSCRIPT_VM_VM_H
#define MCSCRIPT_VM_VM_H

#include <stdint.h>
#include <chunk.h>

typedef struct {
  uint8_t* ip;
  Chunk* chunk;
} VM;

typedef enum {
  INTERPRET_OK,
  COMPILE_ERROR
} InterpretResult;

VM* initVM(Chunk* chunk);
void freeVM(VM* vm);
InterpretResult interpret(VM* vm);


#endif
