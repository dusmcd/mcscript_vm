#include <value.h>
#include <chunk.h>
#include <common.h>
#include <vm.h>
#include <memory.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <debug.h>
#include <compiler.h>
#include <parser.h>
#include <object.h>
#include <string.h>
#include <table.h>

void initVM(VM* vm, Chunk* chunk, Compiler* compiler) {
  vm->chunk = chunk;
  vm->stackTop = vm->valueStack;

  vm->objects = NULL;
  vm->compiler = compiler;
  initTable(&vm->globals);
}

void resetVM(VM* vm) {
  vm->stackTop = vm->valueStack;
}

static void freeObject(Obj* obj) {
  switch(obj->type) {
    case OBJ_STRING: {
      ObjString* str = (ObjString*)obj;
      FREE_ARRAY(char, str->str, str->length + 1);
      FREE(ObjString, str);
      break;
    }
  }
}

static void freeObjects(VM* vm) {
  Obj* obj = vm->objects;
  int count = 0;

  while (obj != NULL) {
    Obj* next = obj->next;
    freeObject(obj);
    count++;
    obj = next;
  }

#ifdef DEBUG_STACK_TRACE
  printf("%d objects freed\n", count);
#endif
}


void freeVM(VM* vm) {
  freeObjects(vm);
  freeTable(&vm->globals);
}

static void error(const char* msg) {
  fprintf(stderr, "ERROR: %s\n", msg);
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
          error("both operands must be number types");
          return false;
      }

      switch(op) {
        case OP_ADD: 
          NUM_OP(+, NUMBER_VAL, double);
          break;
        case OP_SUBTRACT: NUM_OP(-, NUMBER_VAL, double); break;
        case OP_MULTIPLY: NUM_OP(*, NUMBER_VAL, double); break;
        case OP_DIVIDE: NUM_OP(/, NUMBER_VAL, double); break;
        default: {
          error("operand not supported for binary operation");
          return false;
        }
      } 
      return true;
    }

   case VAL_BOOL: {
    if (peek(vm, 1).type != VAL_NUMBER || peek(vm, 2).type != VAL_NUMBER) {
      error("both operands must be number types");
      return false;
    }
    switch(op) {
      case OP_LESS: NUM_OP(<, BOOL_VAL, bool); break;
      case OP_GREATER: NUM_OP(>, BOOL_VAL, bool); break;
      default:{
        error("operand not supported for comparison");
        return false;
      } 
    }
    return true;
   }
   
  default: {
    // handle error
    char buff[100];
    snprintf(buff, sizeof(buff), "value type %d not supported as binary operand\n", 
        valType);
    error(buff);
    return false;
  }
 }
#undef NUM_OP
  return true;
}

static bool evalEquals(VM* vm) {
  
  if (peek(vm, 1).type != peek(vm, 2).type) {
    return false;
  }

  Value a = pop(vm);
  Value b = pop(vm);

  switch(a.type) {
    case VAL_NUMBER:
      push(vm, BOOL_VAL(AS_NUMBER(a) == AS_NUMBER(b)));
      break;
    case VAL_BOOL:
      push(vm, BOOL_VAL(AS_BOOL(a) == AS_BOOL(b)));
      break;
    case VAL_NULL:
      push(vm, BOOL_VAL(true));
      break;
    case VAL_OBJ:
      push(vm, BOOL_VAL(strcmp(AS_CSTRING(a), AS_CSTRING(b)) == 0));
      break;
  }
  return true;
}

static bool concatenate(VM* vm) {
  if (!(IS_OBJ(peek(vm, 1))) || !(IS_OBJ(peek(vm, 2)))) {
    error("both types must be objects");
    return false;
  }
  if (OBJ_TYPE(peek(vm, 1)) != OBJ_STRING && OBJ_TYPE(peek(vm, 2)) != OBJ_STRING) {
    // handle error
    error("both types must be strings");
    return false;
  }

  ObjString* b = AS_STRING(pop(vm));
  ObjString* a = AS_STRING(pop(vm));
  int size = a->length + b->length;

  char* buff = strcat(a->str, b->str);
  char* str = ALLOCATE(char, size + 1);
  strcpy(str, buff);
  ObjString* obj = allocateString(vm, str);
  push(vm, OBJ_VAL(obj));

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
        if (peek(vm, 1).type == VAL_OBJ ) {
          if (concatenate(vm)) break;
          return RUNTIME_ERROR;
        }

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
      case OP_LESS: {
        if (binaryOp(vm, VAL_BOOL, OP_LESS)) break;
        return RUNTIME_ERROR;
      }
      case OP_GREATER: {
        if (binaryOp(vm, VAL_BOOL, OP_GREATER)) break;
        return RUNTIME_ERROR;
      }
      case OP_EQUAL: {
        if (evalEquals(vm)) break;
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
      case OP_DEFINE_GLOBAL: {
        ObjString* name = AS_STRING(peek(vm, 2));
        Value val = peek(vm, 1);
        tableSet(&vm->globals, name, val);
        pop(vm);
        pop(vm);
        break;
      }
      case OP_GET_GLOBAL: {
        ObjString* name = AS_STRING(peek(vm, 1));
        Value val;
        bool found = tableGet(&vm->globals, name, &val);
        if (!found) {
          error("undefined identifier");
          return RUNTIME_ERROR;
        }
        pop(vm);
        push(vm, val);
        break;
      }
      case OP_GET_LOCAL: {
        Value slot = peek(vm, 1);
        pop(vm);
        int index = (int)AS_NUMBER(slot);
        push(vm, vm->valueStack[index]);
        break;
      }
      case OP_POP:
        pop(vm);
        break;
      case OP_RETURN:
        if (vm->stackTop > vm->valueStack) {
          printValue(pop(vm));
          printf("\n");
        }
        return INTERPRET_OK;
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
}


InterpretResult interpret(VM* vm, const char* source) {
  Parser parser;
  Statements statements = parse(&parser, source);

  if (!compile(vm, &statements)) {
    freeStatements(&statements);
    return COMPILE_ERROR;
  }
  freeStatements(&statements);
  writeChunk(vm->chunk, OP_RETURN, 999);

  // setting instruction pointer to first instruction in chunk
  vm->ip = vm->chunk->code;
  return run(vm);
}
