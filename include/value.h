#ifndef MCSCRIPT_VM_VALUE_H
#define MCSCRIPT_VM_VALUE_H

#include <stdbool.h>

#define AS_NUMBER(value) value.as.number
#define AS_BOOL(value) value.as.boolean
#define AS_NULL(value) value.as.null

#define NUMBER_VAL(num) (Value){VAL_NUMBER, {.number = num}}
#define BOOL_VAL(boolean) (Value){VAL_BOOL, {.boolean = boolean}}
#define NULL_VAL (Value){VAL_NULL, {.null = 0}}


/**
 * the set of all possible value types the
 * interpreter can have
 */
typedef enum {
  VAL_NUMBER,
  VAL_BOOL,
  VAL_NULL 
} ValueType;

typedef union {
  double number; // => VAL_NUMBER
  bool boolean; // => VAL_BOOL
  int null; // => VAL_NULL
} ValueData;

/**
 * generic container for all literals
 */
typedef struct {
  ValueType type;
  ValueData as;
} Value;

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

void printValue(Value val);

#endif
