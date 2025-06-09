#ifndef MCSCRIPT_VM_TEST_PARSER_TEST_H
#define MCSCRIPT_VM_TEST_PARSER_TEST_H

#define NUM_TESTS 100

typedef struct {
  int count;
  const char* tests[NUM_TESTS];
} Test;

void testParser();

#endif
