#include <parser_test.h>
#include <parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static bool testNumber(Statement statement, double expected) {
  if (statement.type != STMT_EXPR) {
    fprintf(stderr, "statement is not STMT_EXPR\n");
    return false;
  }

  Expression expr = statement.data.expressionStmt.expression;
  if (expr.type != EXPR_NUMBER) {
    fprintf(stderr, "expression is not EXPR_NUMBER\n");
    return false;
  }

  if (expr.data.number.value != expected) {
    fprintf(stderr, "wrong value. expected=%f got=%f\n",
        expected, expr.data.number.value);
    return false;
  }

  return true;
}

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
    double expected = strtod(tests.tests[i], NULL);
    if (!testNumber(statement, expected)) {
      return;
    }
  }

  printf("testNumberExpression() passed\n");
}

void testParser() {
  printf("=== Parser Tests ===\n");
  testReturnStmt();
  testNumberExpression();
  printf("\n");
}
