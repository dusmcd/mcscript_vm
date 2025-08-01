#ifndef MCSCRIPT_VM_TEST_PARSER_TEST_H
#define MCSCRIPT_VM_TEST_PARSER_TEST_H

#include <scanner.h>

#define NUM_TESTS 100

typedef struct {
  int count;
  const char* tests[NUM_TESTS];
  int expectedNums[NUM_TESTS];
  TokenType expectedOp[NUM_TESTS];
  const char* expectedArgs[NUM_TESTS];
} Test;

void testParser();

#endif
