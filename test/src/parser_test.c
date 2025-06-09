#include <parser_test.h>
#include <parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static bool testNumber(Expression expr, double expected) {
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
  
  Test tests = {.count = 2, .tests = {"return;", "return 10;"}};
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

    ReturnStatement rs = statement.data.returnStmt;
    if (rs.expression.type != EXPR_NULL && !testNumber(rs.expression, 10)) {
      return;
    }
    freeStatements(&stmts);
  }

  printf("testReturnStmt() passed\n");
}

static void testNumberExpression() {
  Parser parser;
  Test tests = {.count = 3, .tests = {"10", "11", "10.12"}};
  for (int i = 0; i < tests.count; i++) {
    Statements stmts = parse(&parser, tests.tests[i]);
    if (stmts.count != 1) {
      fprintf(stderr, "stmts does not contain 1 statement. got=%d\n",
          stmts.count);
      return;
    }

    Statement statement = stmts.stmts[0];
    if (statement.type != STMT_EXPR) {
      fprintf(stderr, "statement is not STMT_EXPR\n");
      return;
    }

    double expected = strtod(tests.tests[i], NULL);
    if (!testNumber(statement.data.expressionStmt.expression, expected)) {
      return;
    }
    freeStatements(&stmts);
  }

  printf("testNumberExpression() passed\n");
}

static void testVarStmt() {
  Parser parser;
  Test test = {.count = 2, .tests = {"var x;", "var x = 10;"}};

  for (int i = 0; i < test.count; i++) {
    const char* source = test.tests[i];
    Statements stmts = parse(&parser, source);

    if (stmts.count != 1) {
      fprintf(stderr, "stmts does not contain 1 statement. got=%d\n",
          stmts.count);
      return;
    }

    Statement statement = stmts.stmts[0];
    if (statement.type != STMT_VAR) {
      fprintf(stderr, "statement is not STMT_VAR\n");
      return;
    }

    Token token = statement.data.varStmt.token;
    if (memcmp(token.start, "var", token.length) != 0) {
      fprintf(stderr, "wrong token literal. expected='var' got=%.*s",
          token.length, token.start);
      return;
    }

    Expression expr = statement.data.varStmt.value;
    if (expr.type != EXPR_NULL && !testNumber(expr, 10)) {
      return;
    }

    freeStatements(&stmts);
  }

  printf("testVarStmt() passed\n");
}

void testParser() {
  printf("=== Parser Tests ===\n");
  testReturnStmt();
  testVarStmt();
  testNumberExpression();
  printf("\n");
}
