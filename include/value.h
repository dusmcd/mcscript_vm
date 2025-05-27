#ifndef MCSCRIPT_VM_VALUE_H
#define MCSCRIPT_VM_VALUE_H


typedef double Value;

typedef struct {
  int count;
  int capacity;
  Value* data;
} ValueArray;

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value val);
void freeValueArray(ValueArray* array);

#endif
