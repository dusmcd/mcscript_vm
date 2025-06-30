#ifndef MCSCRIPT_VM_TEST_TABLE_TEST_H
#define MCSCRIPT_VM_TEST_TABLE_TEST_H

#include <value.h>

#define NUM_TESTS 100

typedef struct {
  int count;
  const char* keys[NUM_TESTS];
  Value vals[NUM_TESTS];
} TableTest;


void testTable();

#endif
