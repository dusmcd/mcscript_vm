#include <value.h>
#include <memory.h>
#include <stdio.h>
#include <object.h>


void initValueArray(ValueArray* array) {
  array->data = NULL;
  array->capacity = 0;
  array->count = 0;
}

void writeValueArray(ValueArray* array, Value val) {
  if (array->capacity < array->count + 1) {
    int oldCapacity = array->capacity;
    array->capacity = GROW_CAPACITY(oldCapacity);
    array->data = GROW_ARRAY(Value, array->data, oldCapacity, array->capacity);
  }

  array->data[array->count] = val;
  array->count++;
}

void freeValueArray(ValueArray* array) {
  FREE_ARRAY(Value, array->data, array->capacity);
  initValueArray(array);
}

static void printObject(ObjType type, Value val) {
  switch(type) {
    case OBJ_STRING: {
      char* str = AS_CSTRING(val);
      printf("%s", str);
      break;
    }
    case OBJ_FUNCTION: {
      ObjFunction* func = AS_FUNC(val);
      printf("function<%s>", func->name->str);
      break;
    }
  }
}

void printValue(Value val) {
  switch (val.type) {
    case VAL_NUMBER:
      printf("%g", AS_NUMBER(val));
      break;
    case VAL_BOOL:
      printf("%s", AS_BOOL(val) ? "true" : "false");
      break;
    case VAL_NULL:
      printf("%s", "null");
      break;
    case VAL_OBJ:
      printObject(OBJ_TYPE(val), val);
      break;
  }
}
