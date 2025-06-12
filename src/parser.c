#include <scanner.h>
#include <parser.h>
#include <stdbool.h>
#include <stdio.h>
#include <memory.h>

static Expression number(Parser*, Scanner*);
static Expression unary(Parser*, Scanner*);
static Expression binary(Parser*, Scanner*, Expression);
static Expression parseExpression(Parser*, Scanner*, Precedence);
static Expression grouped(Parser*, Scanner*);
static bool expect(Parser*, Scanner*, TokenType);

static void error(Parser* parser, const char* msg) {
  fprintf(stderr, "[line %d]: Error: %s\n", parser->previous.line, msg);
}

static void advance(Parser* parser, Scanner* scanner) {
  parser->previous = parser->current;
  parser->current = scanToken(scanner);
}


const ParserRule rules[] = {
  [TOKEN_NUMBER] = {number, NULL, PREC_NONE},
  [TOKEN_MINUS] = {unary, binary, PREC_TERM},
  [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
  [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
  [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
  [TOKEN_BANG] = {unary, NULL, PREC_UNARY},
  [TOKEN_LEFT_PAREN] = {grouped, NULL, PREC_NONE},
  [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE}
};

static ParserRule getRule(TokenType type) {
  return rules[type];
}

static Expression grouped(Parser* parser, Scanner* scanner) {
  Group group = {.token = parser->previous};

  advance(parser, scanner);
  Expression innerExpr = parseExpression(parser, scanner, getRule(parser->previous.type).precedence);
  Expression* innerExprP = (Expression*)malloc(sizeof(Expression));
  *innerExprP = innerExpr;

  group.expr = innerExprP;

  // jump over right paren
  if (!expect(parser, scanner, TOKEN_RIGHT_PAREN)) {
    // handle error
    error(parser, "unexpected token");
    Expression expr = {.type = EXPR_ERROR};
    return expr;
  }
  
  Expression expr = {.type = EXPR_GROUP};
  expr.data.group = group;

  return expr;
}

static Expression binary(Parser* parser, Scanner* scanner, Expression left) {
  Infix infix = {.token = parser->previous, .operator = parser->previous.type};
  Expression* leftP = (Expression*)malloc(sizeof(Expression));
  *leftP = left;
  infix.left = leftP;

  advance(parser, scanner);

  Expression right = parseExpression(parser, scanner, getRule(parser->previous.type).precedence);
  Expression* rightP = (Expression*)malloc(sizeof(Expression));
  *rightP = right;
  infix.right = rightP;

  Expression expr = {.type = EXPR_INFIX};
  expr.data.infix = infix;

  return expr;
}

static Expression unary(Parser* parser, Scanner* scanner) {
  Prefix prefix = {.token = parser->previous, .operator = parser->previous.type};
  advance(parser, scanner);

  Expression* prefixExpr = (Expression*)malloc(sizeof(Expression));
  *prefixExpr = parseExpression(parser, scanner, PREC_UNARY);

  prefix.expression = prefixExpr;

  Expression expr = {.type = EXPR_PREFIX};
  expr.data.prefix = prefix;

  return expr;
}

static Expression number(Parser* parser, Scanner* scanner) {
  Number number = {.token = parser->previous};

  char* buff = (char*)malloc(number.token.length + 1);
  snprintf(buff, sizeof(buff), "%.*s", number.token.length, number.token.start);
  buff[number.token.length] = '\0';
  double value = strtod(buff, NULL);
  number.value = value;

  Expression expr = {.type = EXPR_NUMBER};
  expr.data.number = number;

  return expr;
}

static void initParser(Parser* parser, Scanner* scanner) {
  advance(parser, scanner);
  advance(parser, scanner);
}

static bool expect(Parser* parser, Scanner* scanner, TokenType type) {
  if (parser->current.type == type) {
    advance(parser, scanner);
    return true;
  }

  return false;
}

static Precedence peekPrecedence(TokenType type) {
  return getRule(type).precedence;
}

static Expression parseExpression(Parser* parser, Scanner* scanner, Precedence prec) {
  ParserRule rule = getRule(parser->previous.type);
  PrefixFn prefixFn = rule.prefix;
  if (prefixFn == NULL) {
    // handle error
    char buff[256];
    snprintf(buff, sizeof(buff), "no parser function for token %d", parser->previous.type);
    error(parser, buff);
    Expression expr = {.type = EXPR_ERROR};
    return expr;
  }

  Expression expr = prefixFn(parser, scanner);

  while (prec < peekPrecedence(parser->current.type) && parser->current.type != TOKEN_EOF) {
    ParserRule infixRule = getRule(parser->current.type);
    InfixFn infixFn = infixRule.infix;

    advance(parser, scanner);

    expr = infixFn(parser, scanner, expr);
  }

  return expr;
}

static ReturnStatement parseReturnStatement(Parser* parser, Scanner* scanner) {
  ReturnStatement rs;
  rs.token = parser->previous;
  if (parser->current.type == TOKEN_SEMICOLON) {
    Expression expr = {.type = EXPR_NULL};
    rs.expression = expr;
    return rs;
  }

  advance(parser, scanner);
  rs.expression = parseExpression(parser, scanner, getRule(parser->previous.type).precedence);
  return rs;
}

static VarStatement parseVarStatement(Parser* parser, Scanner* scanner) {
  VarStatement vs = {.token = parser->previous};

  if (!expect(parser, scanner, TOKEN_IDENTIFIER)) {
    // handle error
  }

  Identifier name = {.length = parser->previous.length, .start = parser->previous.start};
  vs.name = name;

  if (parser->current.type == TOKEN_EQUAL) {
    // jump over equals sign
    advance(parser, scanner);
    advance(parser, scanner);
    vs.value = parseExpression(parser, scanner, getRule(parser->previous.type).precedence);
  } else {
    vs.value = (Expression){.type = EXPR_NULL};
  }

  return vs;
}

static ExpressionStatement parseExpressionStatement(Parser* parser, Scanner* scanner) {
  ExpressionStatement es = {.token = parser->previous};
  es.expression = parseExpression(parser, scanner, PREC_NONE);
  return es;
}

static Statement parseStatement(Parser* parser, Scanner* scanner) {
  Statement stmt;
  switch(parser->previous.type) {
    case TOKEN_RETURN: {
      ReturnStatement rs = parseReturnStatement(parser, scanner);
      stmt.type = STMT_RETURN;
      stmt.data.returnStmt = rs;
      break;
    }
    case TOKEN_VAR: {
      VarStatement vs = parseVarStatement(parser, scanner);
      stmt.type = STMT_VAR;
      stmt.data.varStmt = vs;
      break;
    }
    case TOKEN_SEMICOLON:
      stmt.type = STMT_NULL;
      break;
    default: {
      ExpressionStatement es = parseExpressionStatement(parser, scanner);
      stmt.type = STMT_EXPR;
      stmt.data.expressionStmt = es;
      break;
    }
  }

  return stmt;
}

static void append(Statements* statements, Statement stmt) {
  if (statements->capacity < statements->count + 1) {
    int oldCapacity = statements->capacity;
    statements->capacity = GROW_CAPACITY(statements->capacity);
    statements->stmts = GROW_ARRAY(Statement, statements->stmts, oldCapacity, statements->capacity);
  }

  statements->stmts[statements->count] = stmt;
  statements->count++;
}


Statements parse(Parser* parser, const char* source) {
  Scanner scanner;
  initScanner(&scanner, source);
  initParser(parser, &scanner);

  Statements statements = {
    .capacity = 0,
    .count = 0,
    .stmts = NULL
  };

  while (true) {
    Statement stmt = parseStatement(parser, &scanner);
    if (stmt.type != STMT_NULL) {
      append(&statements, stmt);    
    }

    if (parser->current.type == TOKEN_EOF) {
      break;
    }

    advance(parser, &scanner);
  }

  freeScanner(&scanner);

  return statements;
}


void freeStatements(Statements* statements) {
  if (statements->stmts != NULL) {
    free(statements->stmts);
  }

  statements->stmts = NULL;
  statements->capacity = 0;
  statements->count = 0;
}

void freePrefix(Prefix* prefix) {
  if (prefix->expression != NULL) {
    free(prefix->expression);
  }

  prefix->expression = NULL;
}

void freeInfix(Infix* infix) {
  if (infix->left != NULL) {
    free(infix->left);
    infix->left = NULL;
  }

  if (infix->right != NULL) {
    free(infix->right);
    infix->right = NULL;
  }
}

void freeGrouped(Group* group) {
  if (group->expr != NULL) {
    free(group->expr);
    group->expr = NULL;
  }
}

void freeExpression(Expression* expr) {
  switch(expr->type) {
    case EXPR_PREFIX:
      freeExpression(expr->data.prefix.expression);
      freePrefix(&expr->data.prefix);
      break;
    case EXPR_INFIX:
      freeExpression(expr->data.infix.left);
      freeExpression(expr->data.infix.right);
      freeInfix(&expr->data.infix);
      break;
    case EXPR_GROUP:
      freeExpression(expr->data.group.expr);
      freeGrouped(&expr->data.group);
      break;
    default:
      return;
  }
}
