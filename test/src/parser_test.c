#include <parser_test.h>
#include <parser.h>
#include <stdio.h>

void testParser() {
  const char* source = "var x = 10;";
  Parser parser;
  Statements stmts = parse(&parser, source);
  printf("Parser test running\n");
}
