#include <value.h>
#include <memory.h>
#include <stdio.h>


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

void printValue(Value val) {
  printf("%g", val);
}
