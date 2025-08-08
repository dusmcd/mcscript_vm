#include <stdint.h>
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
#include <native.h>

const char* funcName = NULL;

static void setReturnVal(VM* vm, Value val) {
  const char* temp = "return";
  int length = strlen(temp) + 1;
  char* str = ALLOCATE(char, length);
  strcpy(str, temp);
  str[length] = '\0';

  ObjString* key = allocateString(vm, str);
  tableSet(&vm->globals, key, val);
}

void initVM(VM* vm, Compiler* compiler) {
  vm->stackTop = vm->valueStack;
  vm->compiler = compiler;

  vm->objects = NULL;
  initTable(&vm->globals);
  setReturnVal(vm, NULL_VAL);
  defineNatives(vm);

}

static void printFuncName(const CallFrame* frame) {
  if (funcName == NULL) {
    funcName = "script";
    printf("<%s>\n", funcName);
    return;
  }

  const char* current = frame->func->name == NULL ? "script" : frame->func->name->str;
  if (strcmp(funcName, current) != 0) {
    funcName = current;
    printf("<%s>\n", funcName);
    return;
  }
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
    case OBJ_FUNCTION: {
      ObjFunction* func = (ObjFunction*)obj;
      if (func->name != NULL) {
        freeObject((Obj*)func->name);
      }
      FREE(ObjFunction, func);
      freeChunk(&CHUNK((*func)));
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
  Value* top = vm->stackTop;
  return *top;
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

static bool call(VM* vm, ObjFunction* func, uint8_t callArgs) {
  if (func->numArgs != callArgs) {
    error("wrong number of args");
    pop(vm); // popping function object off stack
    return false;
  }
  pop(vm); // popping function object off stack

  vm->frameCount++;
  if (vm->frameCount > FRAMES_MAX) {
    error("stack overflow");
    return false;
  }

  CallFrame* newFrame = &vm->frame[vm->frameCount - 1];
  newFrame->func = func;
  newFrame->ip = newFrame->func->chunk.code;
  newFrame->basePointer = vm->stackTop - newFrame->func->numArgs;

  return true;
}


static int resolveLocal(VM* vm, const CallFrame* frame) {
  int index = (int)AS_NUMBER(pop(vm));
  for (Value* current = frame->basePointer + index; current < vm->stackTop; current++) {
    if (current->isLocal) {
      break;
    }
    index++;
  }
  return index;
}

static bool callValue(VM* vm, int callArgs) {
  Value val = peek(vm, 1);
  if (IS_OBJ(val)) {
    switch(AS_OBJ(val)->type) {
      case OBJ_FUNCTION: {
        ObjFunction* func = AS_FUNC(val);
        return call(vm, func, callArgs);
      }
      case OBJ_NATIVE: {
        ObjNative* native = AS_NATIVE(val);
        NativeFunc func = native->func;
        pop(vm); // pop off native object from stack
        func(callArgs, vm->stackTop - callArgs);
        vm->stackTop -= callArgs;
        break;
      }
      default:
        error("value being called must be a function object");
        return false;

    }
  } else {
      error("value being called must be a function object");
      return false;
  }

  return true;
}

static InterpretResult run(VM* vm) {

  CallFrame* frame = &vm->frame[vm->frameCount - 1];
#define READ_BYTE() *(frame->ip++)
#define READ_CONSTANT() \
  (frame->func->chunk.constants.data[READ_BYTE()])
#define READ_SHORT() \
  (uint16_t)((frame->ip[0] << 8) | frame->ip[1])

  while(true) {
    frame = &vm->frame[vm->frameCount - 1];

#ifdef DEBUG_STACK_TRACE
  printFuncName(frame);
  printf("[ ");
  for (Value* val = vm->valueStack; val < vm->stackTop; val++) {
    printValue(*val);
    printf(" ");
  }
  printf("]\n");
  disassembleInstruction(&frame->func->chunk, (int)(frame->ip - frame->func->chunk.code));
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
        ObjString* name = AS_STRING(peek(vm, 1));
        Value val = peek(vm, 2);
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
          error("unexpected identifier");
          return RUNTIME_ERROR;
        }
        pop(vm);
        push(vm, val);
        break;
      }
      case OP_SET_GLOBAL: {
        ObjString* name = AS_STRING(peek(vm, 1));
        bool isNewKey = tableSet(&vm->globals, name, peek(vm, 2));
        if (isNewKey) {
          error("unexpected identifier");
          return RUNTIME_ERROR;
        }
        pop(vm);
        pop(vm);
        break;
      }
      case OP_GET_LOCAL: {
        int index = resolveLocal(vm, frame);
        push(vm, frame->basePointer[index]);
        break;
      }
      case OP_SET_LOCAL: {
        int index = resolveLocal(vm, frame);
        Value newVal = pop(vm);
        newVal.isLocal = true;
        frame->basePointer[index] = newVal;
        break;
      }
      case OP_MARK_LOCAL: {
        Value* val = vm->stackTop - 1;
        val->isLocal = true;
        break;
      }
      case OP_CALL: {
        // create a new call frame, the function object
        // should be on the top of the stack
        // the base pointer should be the address of the first arg (if any)
        // the top of the stack will be one past the last arg
        uint8_t callArgs = READ_BYTE();
        if (!callValue(vm, callArgs)) {
          return RUNTIME_ERROR;
        }
        break;
      }
      case OP_JUMP_IF_FALSE: {
        // creating 16-bit integer with arguments
        uint16_t offset = READ_SHORT();
        frame->ip += 2;
        if (isFalsey(peek(vm, 1))) {
          frame->ip += offset;
        }
        break;
      }
      case OP_JUMP_IF_TRUE: {
        uint16_t offset = READ_SHORT();
        frame->ip += 2;
        if (!isFalsey(peek(vm, 1))) {
          frame->ip += offset;
        }
        break;
      }
      case OP_LOOP: {
        uint16_t offset = READ_SHORT();
        frame->ip += 2;
        frame->ip -= offset;
        break;
      }
      case OP_JUMP: {
        uint16_t offset = READ_SHORT();
        frame->ip += 2;
        frame->ip += offset;
        break;
      }
      case OP_POP:
        pop(vm);
        break;
      case OP_RETURN: {
        Value val = pop(vm); // grab return value
        if (vm->frameCount - 1 == 0) {
          return INTERPRET_OK;
        }

        setReturnVal(vm, val);
        vm->frameCount--;
        vm->stackTop = frame->basePointer;
        break;
      }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
}


InterpretResult interpret(VM* vm, const char* source) {
  Parser parser;
  Statements statements = parse(&parser, source);

  if (compile(vm, &statements).hasError) {
    freeStatements(&statements);
    return COMPILE_ERROR;
  }
  freeStatements(&statements);

  vm->frameCount = 1;
  CallFrame* frame = &vm->frame[vm->frameCount - 1];
  frame->basePointer = vm->valueStack;
  frame->func = vm->compiler->func;
  frame->ip = frame->func->chunk.code;

  return run(vm);
}
