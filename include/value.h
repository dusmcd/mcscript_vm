#ifndef MCSCRIPT_VM_VALUE_H
#define MCSCRIPT_VM_VALUE_H


typedef double Value;

/**
 * dynamic array to store literals in source code
 */
typedef struct {
  int count;
  int capacity;
  Value* data;
} ValueArray;

/**
 * initialize empty ValueArray
 */
void initValueArray(ValueArray* array);

/**
 * add a value to the ValueArray
 */
void writeValueArray(ValueArray* array, Value val);

/**
 * free up memory on the heap
 */
void freeValueArray(ValueArray* array);

#endif
