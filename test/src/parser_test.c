#include <parser_test.h>
#include <parser.h>
#include <stdio.h>
#include <string.h>

static void testReturnStmt() {
  Parser parser;
  
  Test tests = {.count = 2, .tests = {"return;", "return 10"}};
  for (int i = 0; i < tests.count; i++) {
    Statements stmts = parse(&parser, tests.tests[i]);
    if (stmts.count != 1) {
      fprintf(stderr, "stmts does not contain 1 statement. got %d\n",
          stmts.count);
      return;
    }
    Statement statement = stmts.stmts[0];
    if (statement.type != STMT_RETURN) {
      fprintf(stderr, "statement is not STMT_RETURN\n");
      return;
    }

    Token token = statement.data.returnStmt.token;
    if (memcmp(token.start, "return", token.length) != 0) {
      fprintf(stderr, "Wrong token literal. Expected='return' got='%.*s'\n",
          token.length, token.start);
      return;
    }
  }

  printf("testReturnStmt() passed\n");
}

static void testNumberExpression() {
  Parser parser;
  Test tests = {.count = 3, .tests = {"10", "11", "10.12"}};
  for (int i = 0; i < tests.count; i++) {
    Statements stmts = parse(&parser, tests.tests[i]);
    if (stmts.count != 1) {
      fprintf(stderr, "stmts does not contain 1 statement. got%d\n",
          stmts.count);
      return;
    }

    Statement statement = stmts.stmts[0];
  }
}

void testParser() {
  testReturnStmt();
}
