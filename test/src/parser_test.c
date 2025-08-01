#include "ast.h"
#include <scanner.h>
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

static void testStringExpressions() {
  Parser parser;
  Test test = {
    .count = 2,
    .tests = {"\"hello world!\";", "\"hello\";"},
    .expectedNums = {strlen("hello world!"), strlen("hello")}
  };

  for (int i = 0; i < test.count; i++) {
    const char* source = test.tests[i];
    Statements stmts = parse(&parser, source);

    if (stmts.count != 1) {
      fprintf(stderr, "stmts does not contain 1 statement got=%d\n",
          stmts.count);
      return;
    }

    Statement statement = stmts.stmts[0];
    if (statement.type != STMT_EXPR) {
      fprintf(stderr, "statement not STMT_EXPR\n");
      return;
    }

    Expression expr = statement.data.expressionStmt.expression;
    if (expr.type != EXPR_STRING) {
      fprintf(stderr, "expression not EXPR_STRING\n");
      return;
    }

    String str = expr.data.string;

    // accounting for opening and closing quotation marks
    int expectedLength = test.expectedNums[i];
    int actualLength = str.token.length - 2;
    if (expectedLength != actualLength) {
      fprintf(stderr, "str wrong length. expected=%d got=%d\n",
          test.expectedNums[i], str.token.length);
      return;
    }

    if (memcmp(str.token.start + 1, source + 1, expectedLength) != 0) {
      fprintf(stderr, "str wrong value expected=%.*s got=%.*s\n",
           expectedLength, source + 1, actualLength, str.token.start + 1);
      return;
    }

    freeStatements(&stmts);
  }

  printf("testStringExpressions() passed\n");
}

static void testErrors() {
  Parser parser;
  Test test = {.count = 1, .tests = {"(5 + 1 * 3;"}};

  for (int i = 0; i < test.count; i++) {
    const char* source = test.tests[i];
    Statements stmts = parse(&parser, source);

    if (stmts.count != 1) {
      fprintf(stderr, "stmts does not contain 1 statement got=%d\n", 
          stmts.count);
      return;
    }

    Statement statement = stmts.stmts[0];
    if (statement.type != STMT_EXPR) {
      fprintf(stderr, "statement is not STMT_EXPR\n");
      return;
    }

    Expression expr = statement.data.expressionStmt.expression;
    if (expr.type != EXPR_ERROR) {
      fprintf(stderr, "expr is not EXPR_ERROR\n");
      return;
    }
  }

  printf("testErrors() passed\n");
}

static void testBooleanExpressions() {
  Parser parser;
  Test test = {.count = 2, .tests = {"true;", "false;"}, .expectedNums = {true, false}};

  for (int i = 0; i < test.count; i++) {
    const char* source = test.tests[i];
    Statements stmts = parse(&parser, source);

    if (stmts.count != 1) {
      fprintf(stderr, "stmts does not contain 1 statement got=%d\n",
          stmts.count);
      return;
    }

    Statement statement = stmts.stmts[0];
    if (statement.type != STMT_EXPR) {
      fprintf(stderr, "statement is not STMT_EXPR\n");
      return;
    }

    Expression expr = statement.data.expressionStmt.expression;
    if (expr.type != EXPR_BOOL) {
      fprintf(stderr, "expr is not EXPR_BOOL\n");
      return;
    }

    Boolean boolean = expr.data.boolean;
    if (boolean.value != test.expectedNums[i]) {
      fprintf(stderr, "wrong value. expected=%d got=%d\n", 
          test.expectedNums[i], boolean.value);
      return;
    }
    freeStatements(&stmts);
  }

  printf("testBooleanExpressions() passed\n");
}

static void testPrefixExpression() {
  Parser parser;

  Test tests = {.count = 3, .tests = {"!5", "!10", "-1000"}, .expectedNums = {5, 10, 1000}};
  for (int i = 0; i < tests.count; i++) {
    const char* source = tests.tests[i];

    Statements stmts = parse(&parser, source);
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

    TokenType expectedOp = source[0] == '!' ? TOKEN_BANG : TOKEN_MINUS;
    Expression expr = statement.data.expressionStmt.expression;
    Expression* innerExp = expr.data.prefix.expression;

    if (expr.type != EXPR_PREFIX) {
      fprintf(stderr, "expression is not EXPR_PREFIX\n");
      freeExpression(&expr);
      return;
    }

    if (expr.data.prefix.operator != expectedOp) {
      fprintf(stderr, "wrong prefix operator\n");
      freeExpression(&expr);
      return;
    }

    if (!testNumber(*innerExp, tests.expectedNums[i])) {
      freeExpression(&expr);
      return;
    }
    freeExpression(&expr);
    freeStatements(&stmts);
  }

  printf("testPrefixExpression() passed\n");
}

static bool testInfixExpression(const Expression* expr, double left, double right, TokenType op) {
    if (expr->type != EXPR_INFIX) {
      fprintf(stderr, "expr->ssion is not EXPR_INFIX\n");
      return false;
    }

    Infix infix = expr->data.infix;
    Expression* leftExpr = infix.left;
    Expression* rightExpr = infix.right;
    TokenType operator = infix.operator;

    if (!testNumber(*leftExpr, left)) {
      return false;
    }

    if (!testNumber(*rightExpr, right)) {
      return false;
    }

    if (infix.operator != op) {
      fprintf(stderr, "wrong operator\n");
      return false;
    }

  return true;
}

static void testInfixExpressions() {
  Parser parser;
  Test tests = {
    .count = 4,
    .tests = {"1 + 1;", "2 * 2;", "3 / 3;", "4 - 4;"},
    .expectedNums = {1, 2, 3, 4},
    .expectedOp = {TOKEN_PLUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_MINUS}
  };

  for (int i = 0; i < tests.count; i++) {
    const char* source = tests.tests[i];
    Statements stmts = parse(&parser, source);

    if (stmts.count != 1) {
      fprintf(stderr, "stmts does not contain 1 statement got=%d\n",
          stmts.count);
      return;
    }

    Statement statement = stmts.stmts[0];
    if (statement.type != STMT_EXPR) {
      fprintf(stderr, "statement is not STMT_EXPR\n");
      return;
    }

    Expression expr = statement.data.expressionStmt.expression;
    if (!testInfixExpression(&expr, tests.expectedNums[i], tests.expectedNums[i], tests.expectedOp[i])) {
      freeExpression(&expr);
      return;
    }
    
    freeExpression(&expr);
    freeStatements(&stmts);
  }

  printf("testInfixExpression() passed\n");
}

static void testGroupExpression() {
  Parser parser;
  Test tests = {.count = 1, .tests = "(1 + 2) * 3;"};

  for (int i = 0; i < tests.count; i++) {
    const char* source = tests.tests[i];
    Statements stmts = parse(&parser, source);

    if (stmts.count != 1) {
      fprintf(stderr, "stmts does not contain 1 statement got=%d\n",
          stmts.count);
      return;
    }

    Statement statement = stmts.stmts[0];

    if (statement.type != STMT_EXPR) {
      fprintf(stderr, "statement is not STMT_EXPR\n");
      return;
    }

    Expression expr = statement.data.expressionStmt.expression;
    if (expr.type != EXPR_INFIX) {
      fprintf(stderr, "top expression is not EXPR_INFIX\n");
      freeExpression(&expr);
      return;
    }

    Expression* left = expr.data.infix.left;
    if (left->type != EXPR_GROUP) {
      fprintf(stderr, "left expression is not EXPR_GROUP\n");
      freeExpression(&expr);
      return;
    }

    Expression* innerGroupExpr = left->data.group.expr;
    if (innerGroupExpr->type != EXPR_INFIX) {
      fprintf(stderr, "innerGroupExp is not EXPR_INFIX\n");
      freeExpression(&expr);
      return;
    }

    if (!testInfixExpression(innerGroupExpr, 1, 2, TOKEN_PLUS)) {
      freeExpression(&expr);
      return;
    }

    Expression* right = expr.data.infix.right;
    if (right->type != EXPR_NUMBER) {
      fprintf(stderr, "right expression is not EXPR_NUMBER\n");
      freeExpression(&expr);
      return;
    }

    if (!testNumber(*right, 3)) {
      freeExpression(&expr);
      return;
    }

    freeExpression(&expr);
    freeStatements(&stmts);
  }

  printf("testGroupExpression() passed\n");
}

static void testReturnStmt() {
  Parser parser;
  
  Test tests = {.count = 2, .tests = {"return;", "return 10;"}};
  for (int i = 0; i < tests.count; i++) {
    Statements stmts = parse(&parser, tests.tests[i]);
    if (stmts.count != 1) {
      fprintf(stderr, "stmts does not contain 1 statement. got=%d\n",
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

static void testBlockStmt() {
  Parser parser;

  Test tests = {.count = 2, .tests = {"{5;}", "{10;}"}, .expectedNums = {5, 10}};
  for (int i = 0; i < tests.count; i++) {
    Statements stmts = parse(&parser, tests.tests[i]);
    if (stmts.count != 1) {
      fprintf(stderr, "stmts does not contain 1 statement. got=%d\n",
          stmts.count);
      return;
    }
    Statement stmt = stmts.stmts[0];
    if (stmt.type != STMT_BLOCK) {
      fprintf(stderr, "statement is not STMT_BLOCK\n");
      return;
    }
    BlockStatement bs = stmt.data.blockStmt;
    Statements inners = bs.stmts;
    if (inners.count != 1) {
      fprintf(stderr, "inner statements does not contain 1 statement. got=%d\n",
          inners.count);
      return;
    }
    Statement inner = inners.stmts[0];
    if (inner.type != STMT_EXPR) {
      fprintf(stderr, "inner is not STMT_EXPR\n");
      return;
    }

    Expression expr = inner.data.expressionStmt.expression;
    if (!testNumber(expr, tests.expectedNums[i])) {
      return;
    }
    freeStatements(&inners);    
    freeStatements(&stmts);    
  }
  puts("testBlockStmt() passed");
}

static void testNumberExpression() {
  Parser parser;
  Test tests = {.count = 3, .tests = {"10;", "11;", "10.12;"}};
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

void testIfStatement() {
  Parser parser;
  Test test = {.count = 2, .tests = {"if(5 == 5) {10;}", "if (false) {10;} else {1;}"}, .expectedNums = {10, 1}};

  for (int i = 0; i < test.count; i++) {
    const char* src = test.tests[i];
    Statements stmts = parse(&parser, src);

    if (stmts.count != 1) {
      fprintf(stderr, "stmts does not contain 1 statement. got=%d\n",
          stmts.count);
      return;
    }

    Statement stmt = stmts.stmts[0];
    if (stmt.type != STMT_IF) {
      fprintf(stderr, "stmt is not STMT_IF\n");
      return;
    }

    IfStatement is = AS_IFSTMT(stmt);
    BlockStatement block = is.block;
    Statements inner = block.stmts;

    if (inner.count != 1) {
      fprintf(stderr, "inner does not contain 1 statement. got=%d\n",
          inner.count);
      return;
    }

    Statement innerStmt = inner.stmts[0];
    if (innerStmt.type != STMT_EXPR) {
      fprintf(stderr, "innerStmt is not STMT_EXPR\n");
      return;
    }

    Expression expr;
    if (i == 0) {
      expr = AS_EXPRSTMT(innerStmt).expression;
      if (is.elseBlock.token.type != TOKEN_NULL) {
        fprintf(stderr, "else block should be null\n");
        return;
      }
    } else if (i == 1) {
      Statements innerElse = is.elseBlock.stmts;
      if (innerElse.count != 1) {
        fprintf(stderr, "inner else statements does not contain 1 statement. got=%d\n",
            innerElse.count);
        return;
      }

      Statement innerElseStmt = innerElse.stmts[0];
      if (innerElseStmt.type != STMT_EXPR) {
        fprintf(stderr, "innerElseStmt is not STMT_EXPR\n");
        return;
      }

      expr = AS_EXPRSTMT(innerElseStmt).expression;
      freeStatements(&innerElse);
    }

    if (!testNumber(expr, test.expectedNums[i])) {
      return;
    }

    freeStatements(&stmts);
    freeStatements(&inner);
    
  }
  puts("testIfStatement() passed");
}

static void testWhileStatement() {
  Parser parser;
  Test test = {.count = 2, .tests = {"while(true){10;}", "while(1 == 1){10;}"}};

  for (int i = 0; i < test.count; i++) {
    const char* src = test.tests[i];
    Statements stmts = parse(&parser, src);

    if (stmts.count != 1) {
      fprintf(stderr, "stmts does not contain 1 statement. got=%d\n",
          stmts.count);
      return;
    }

    Statement stmt = stmts.stmts[0];
    if (stmt.type != STMT_WHILE) {
      fprintf(stderr, "stmt is not STMT_WHILE\n");
      return;
    }

    BlockStatement block = AS_WHILESTMT(stmt).block;
    Statements inner = block.stmts;

    if (inner.count != 1) {
      fprintf(stderr, "inner stmts does not contain 1 statement. got=%d\n",
          inner.count);
      return;
    }

    Statement innerStmt = inner.stmts[0];
    if (innerStmt.type != STMT_EXPR) {
      fprintf(stderr, "innerStmt not STMT_EXPR\n");
      return;
    }

    Expression expr = AS_EXPRSTMT(innerStmt).expression;
    if (!testNumber(expr, 10.0)) {
      return;
    }

    freeStatements(&stmts);
    freeStatements(&inner);
  }

  puts("testWhileStatement() passed");
}

static void testFunctionStatement() {
  Test test = {
    .count = 2,
    .tests = {"function doStuff(a){10;}", "function doStuff(a, b){10;}"},
    .expectedNums = {1, 2},
    .expectedArgs = {"a", "b"}
  };
  Parser parser;

  for (int i = 0; i < test.count; i++) {
    const char* src = test.tests[i];
    Statements stmts = parse(&parser, src);

    if (stmts.count != 1) {
      fprintf(stderr, "stmts does not contain 1 statement. got=%d\n",
          stmts.count);
      return;
    }

    Statement stmt = stmts.stmts[0];
    if (stmt.type != STMT_FUNCTION) {
      fprintf(stderr, "stmt is not STMT_FUNCTION\n");
      return;
    }

    FunctionStatement fs = AS_FUNCSTMT(stmt);

    if (memcmp(fs.name.start, "doStuff", fs.name.length) != 0) {
      fprintf(stderr, "fs.name does not equal doStuff. got=%.*s\n",
          fs.name.length, fs.name.start);
      return;
    }

    if (fs.argCount != test.expectedNums[i]) {
      fprintf(stderr, "wrong number of arguments. expected=%d, got=%d\n",
          test.expectedNums[i], fs.argCount);
      return;
    }

    for (int j = 0; j < test.expectedNums[i]; j++) {
      Identifier arg = fs.args[j];
      if (memcmp(arg.start, test.expectedArgs[j], arg.length) != 0) {
        fprintf(stderr, "arg identifier is wrong. expected=%s, got=%.*s\n",
            test.expectedArgs[j], arg.length, arg.start);
        return;
      }
    }

    BlockStatement block = fs.block;
    Statements inner = block.stmts;

    if (inner.count != 1) {
      fprintf(stderr, "inner does not contain 1 statement. got=%d\n",
          inner.count);
      return;
    }

    Statement innerStmt = inner.stmts[0];
    if (innerStmt.type != STMT_EXPR) {
      fprintf(stderr, "innerStmt is not STMT_EXPR\n");
      return;
    }

    Expression expr = AS_EXPRSTMT(innerStmt).expression;

    if (!testNumber(expr, 10.0)) {
      return;
    }
    freeStatements(&inner);
    freeStatements(&stmts);
  }

  puts("testFunctionStatement() passed");
}

void testParser() {
  printf("=== Parser Tests ===\n");
  testReturnStmt();
  testVarStmt();
  testNumberExpression();
  testPrefixExpression();
  testInfixExpressions();
  testGroupExpression();
  testErrors();
  testBooleanExpressions();
  testStringExpressions();
  testBlockStmt();
  testIfStatement();
  testWhileStatement();
  testFunctionStatement();
  printf("\n");
}
