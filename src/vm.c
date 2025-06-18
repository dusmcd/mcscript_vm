#include "value.h"
#include <chunk.h>
#include <common.h>
#include <vm.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <debug.h>
#include <compiler.h>
#include <parser.h>

void initVM(VM* vm, Chunk* chunk) {
  vm->chunk = chunk;
  vm->stackTop = vm->valueStack;
}

void resetVM(VM* vm) {
  vm->stackTop = vm->valueStack;
}

void push(VM* vm, Value val) {
  *(vm->stackTop) = val;
  vm->stackTop++;
}

static Value peek(VM* vm, int offset) {
  return vm->stackTop[-offset];
}

Value pop(VM* vm) {
  vm->stackTop--;
  return (*vm->stackTop);
}

static bool isFalsey(Value val) {
  return IS_NULL(val) || (IS_BOOL(val) && !AS_BOOL(val));
}

static bool binaryOp(VM* vm, ValueType valType, OpCode op) {
#define NUM_OP(operator, macro, type) \
  do { \
    double b = AS_NUMBER(pop(vm)); \
    double a = AS_NUMBER(pop(vm)); \
    type result = a operator b; \
    Value value = macro(result); \
    push(vm, value); \
  } while(false)

  switch(valType) {
    case VAL_NUMBER: {
      if (peek(vm, 1).type != VAL_NUMBER || peek(vm, 2).type != VAL_NUMBER) {
          return false;
      }

      switch(op) {
        case OP_ADD: 
          NUM_OP(+, NUMBER_VAL, double);
          break;
        case OP_SUBTRACT: NUM_OP(-, NUMBER_VAL, double); break;
        case OP_MULTIPLY: NUM_OP(*, NUMBER_VAL, double); break;
        case OP_DIVIDE: NUM_OP(/, NUMBER_VAL, double); break;
        default: return false;
      } 
      return true;
    }

   case VAL_BOOL: {
    if (peek(vm, 1).type != VAL_NUMBER || peek(vm, 2).type != VAL_NUMBER) {
      return false;
    }
    switch(op) {
      case OP_LESS: NUM_OP(<, BOOL_VAL, bool); break;
      case OP_GREATER: NUM_OP(>, BOOL_VAL, bool); break;
      default: return false;
    }
    return true;
   }

   default: {
      // handle error
   }
 }
#undef NUM_OP
  return true;
}


static InterpretResult run(VM* vm) {

#define READ_BYTE() *(vm->ip++)
#define READ_CONSTANT() \
  (vm->chunk->constants.data[READ_BYTE()])

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
        if (binaryOp(vm, VAL_NUMBER, OP_ADD)) break;
        return RUNTIME_ERROR;
      }
      case OP_SUBTRACT: {
        if (binaryOp(vm, VAL_NUMBER, OP_SUBTRACT)) break;
        return RUNTIME_ERROR;
      }
      case OP_MULTIPLY: {
        if (binaryOp(vm, VAL_NUMBER, OP_MULTIPLY)) break;
        return RUNTIME_ERROR;
      }
      case OP_DIVIDE: {
        if (binaryOp(vm, VAL_NUMBER, OP_DIVIDE)) break;
        return RUNTIME_ERROR;
      }
      case OP_NEGATE: {
        if (peek(vm, 1).type != VAL_NUMBER) {
          resetVM(vm);
          return RUNTIME_ERROR;
        }
        Value val = NUMBER_VAL(-AS_NUMBER(pop(vm)));
        push(vm, val);
        break;
      }
      case OP_NOT:
        push(vm, BOOL_VAL(isFalsey(pop(vm))));
        break;
      case OP_TRUE: {
        Value val = BOOL_VAL(true);
        push(vm, val);
        break;
      }
      case OP_FALSE: {
        Value val = BOOL_VAL(false);
        push(vm, val);
        break;
      }
      case OP_NULL: {
        Value val = NULL_VAL;
        push(vm, val);
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
}


InterpretResult interpret(VM* vm, const char* source) {
  Parser parser;
  Statements statements = parse(&parser, source);

  if (!compile(vm->chunk, &statements)) {
    freeStatements(&statements);
    return COMPILE_ERROR;
  }
  freeStatements(&statements);
  writeChunk(vm->chunk, OP_RETURN, 999);

  // setting instruction pointer to first instruction in chunk
  vm->ip = vm->chunk->code;
  return run(vm);
}
