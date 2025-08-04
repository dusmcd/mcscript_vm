#include <ast.h>
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
static Expression boolean(Parser*, Scanner*);
static Expression identifier(Parser*, Scanner*);
static void append(Statements* statements, Statement stmt);
static Statement parseStatement(Parser* parser, Scanner* scanner);

static void error(Parser* parser, const char* msg) {
  fprintf(stderr, "[line %d]: Error: %s\n", parser->previous.line, msg);
}

static void advance(Parser* parser, Scanner* scanner) {
  parser->previous = parser->current;
  parser->current = scanToken(scanner);
}

static Expression string(Parser* parser, Scanner* scanner) {
  String string = {.token = parser->previous};
  Expression expr = {.data = {.string = string}, .type = EXPR_STRING};

  return expr;
}

static Expression identifier(Parser* parser, Scanner* scanner) {
  Identifier ident = {
    .length = parser->previous.length,
    .start = parser->previous.start,
    .token = parser->previous
  };
  Expression expr = {.data = {.identifier = ident}, .type = EXPR_IDENT};

  return expr;
}

static bool parseCallParams(Parser* parser, Scanner* scanner, CallExpression* ce) {
  ce->capacity = GROW_CAPACITY(0);
  ce->args = GROW_ARRAY(Expression, ce->args, 0, ce->capacity);

  Expression expr = parseExpression(parser, scanner, PREC_NONE);
  if (expr.type == EXPR_ERROR) return false;
  ce->args[ce->argCount++] = expr;
  
  while (parser->current.type == TOKEN_COMMA) {
    advance(parser, scanner);
    advance(parser, scanner);

    if (ce->capacity < ce->argCount + 1) {
      int oldCapacity = ce->capacity;
      ce->capacity = GROW_CAPACITY(oldCapacity);
      ce->args = GROW_ARRAY(Expression, ce->args, oldCapacity, ce->capacity);
    }

    expr = parseExpression(parser, scanner, PREC_NONE);
    if (expr.type == EXPR_ERROR) {
      return false;
    }
    ce->args[ce->argCount++] = expr;
  }

  return true;
}

static Expression call(Parser* parser, Scanner* scanner, Expression expr) {
  CallExpression ce = {
    .token = parser->previous,
    .name = expr.data.identifier,
    .argCount = 0
  };

  if (parser->current.type != TOKEN_RIGHT_PAREN) {
    advance(parser, scanner);
    if (!parseCallParams(parser, scanner, &ce)) {
      return (Expression){.type = EXPR_ERROR};
    }
  }

  // consume closing paren
  advance(parser, scanner);

  Expression resultExpr = {.type = EXPR_CALL, .data = {.call = ce}};
  return resultExpr;
}

const ParserRule rules[] = {
  [TOKEN_NUMBER] = {number, NULL, PREC_NONE},
  [TOKEN_MINUS] = {unary, binary, PREC_TERM},
  [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
  [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
  [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
  [TOKEN_BANG] = {unary, NULL, PREC_UNARY},
  [TOKEN_LEFT_PAREN] = {grouped, call, PREC_CALL},
  [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
  [TOKEN_TRUE] = {boolean, NULL, PREC_NONE},
  [TOKEN_FALSE] = {boolean, NULL, PREC_NONE},
  [TOKEN_GREATER] = {NULL, binary, PREC_COMPARISON},
  [TOKEN_LESS] = {NULL, binary, PREC_COMPARISON},
  [TOKEN_EQUAL_EQUAL] = {NULL, binary, PREC_EQUALITY},
  [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
  [TOKEN_BANG_EQUAL] = {NULL, binary, PREC_EQUALITY},
  [TOKEN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISON},
  [TOKEN_AND] = {NULL, binary, PREC_AND},
  [TOKEN_OR] = {NULL, binary, PREC_OR},
  [TOKEN_STRING] = {string, NULL, PREC_NONE},
  [TOKEN_IDENTIFIER] = {identifier, NULL, PREC_NONE},
  [TOKEN_ILLEGAL] = {NULL, NULL, PREC_NONE},
  [TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE}
};

static ParserRule getRule(TokenType type) {
  return rules[type];
}

static Expression boolean(Parser* parser, Scanner* scanner) {
  Boolean boolean = {.token = parser->previous};
  boolean.value = parser->previous.type == TOKEN_TRUE ? true : false;
  Expression expr = {.type = EXPR_BOOL, .data = {.boolean = boolean}};
  return expr;
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

  // getting the precedence of the operator
  Precedence prec = getRule(parser->previous.type).precedence;
  advance(parser, scanner);

  Expression right = parseExpression(parser, scanner, prec);
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
  free(buff);

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

static Identifier createName(Parser* parser) {
  Identifier name = {
    .length = parser->previous.length,
    .start = parser->previous.start,
    .token = parser->previous
  };

  return name;
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
    error(parser, "expected identifier");
    return vs;
  }

  vs.name = createName(parser);

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

static BlockStatement parseBlockStatement(Parser* parser, Scanner* scanner) {
  BlockStatement bs = {.token = parser->previous};
  Statements statements = {
    .count = 0,
    .capacity = 0,
    .stmts = NULL
  };
  advance(parser, scanner);

  while (true) {
    Statement stmt = parseStatement(parser, scanner);
    if (stmt.type != STMT_NULL) {
      append(&statements, stmt);    
    }

    if (parser->current.type == TOKEN_EOF) {
      error(parser, "expected closing brace");
      break;
    }

    if (parser->current.type == TOKEN_RIGHT_BRACE) {
      advance(parser, scanner);
      break;
    }

    advance(parser, scanner);
  }

  bs.stmts = statements;
  return bs;
}

static IfStatement parseIfStatement(Parser* parser, Scanner* scanner) {
  IfStatement is = {.token = parser->previous};

  if (!expect(parser, scanner, TOKEN_LEFT_PAREN)) {
    error(parser, "expected open paren");
    return is;
  }

  // consume the left paren
  advance(parser, scanner);

  Expression booleanExpr = parseExpression(parser, scanner, PREC_NONE);
  if (booleanExpr.type == EXPR_ERROR) return is;
  is.condition = booleanExpr;

  if (!expect(parser, scanner, TOKEN_RIGHT_PAREN)) {
    error(parser, "expected closing paren");
    return is;
  }

  if (!expect(parser, scanner, TOKEN_LEFT_BRACE)) {
    error(parser, "expected opening brace");
    return is;
  }

  is.block = parseBlockStatement(parser, scanner);

  if (parser->current.type == TOKEN_ELSE) {
    // consume else token
    advance(parser, scanner); 
    advance(parser, scanner);
    is.elseBlock = parseBlockStatement(parser, scanner);
  } else {
    is.elseBlock = (BlockStatement){.token = (Token){.type = TOKEN_NULL}};
  }


  return is;
}

static WhileStatement parseWhileStatement(Parser* parser, Scanner* scanner) {
  WhileStatement ws = {.token = parser->previous};

  if (!expect(parser, scanner, TOKEN_LEFT_PAREN)) {
    error(parser, "expected opening paren");
    return (WhileStatement){.token = {.type = TOKEN_NULL}};
  }

  // consume opening paren
  advance(parser, scanner);

  ws.condition = parseExpression(parser, scanner, PREC_NONE);

  if (ws.condition.type == EXPR_ERROR) {
    return (WhileStatement){.token = {.type = TOKEN_NULL}};
  }

  if (!expect(parser, scanner, TOKEN_RIGHT_PAREN)) {
    error(parser, "expected closing paren");
    return (WhileStatement){.token = {.type = TOKEN_NULL}};
  }
  if (!expect(parser, scanner, TOKEN_LEFT_BRACE)) {
    error(parser, "expected opening brace");
    return (WhileStatement){.token = {.type = TOKEN_NULL}};
  }

  ws.block = parseBlockStatement(parser, scanner);

  return ws;
}

static AssignStatement parseAssignStatement(Parser* parser, Scanner* scanner) {
  AssignStatement as = {.token = parser->previous, .name = createName(parser)};

  if (!expect(parser, scanner, TOKEN_EQUAL)) {
    error(parser, "expected assignment operator");
    return (AssignStatement){.token = {.type = TOKEN_NULL}};
  }

  // consume equal token
  advance(parser, scanner);

  as.value = parseExpression(parser, scanner, PREC_NONE);
  if (as.value.type == EXPR_ERROR) {
    return (AssignStatement){.token = {.type = TOKEN_NULL}};
  }

  return as;
}
static FunctionStatement parseFunctionStatement(Parser* parser, Scanner* scanner) {
  FunctionStatement fs = {.token = parser->previous, .argCount = 0};
  // consume function keyword
  advance(parser, scanner);

  fs.name = createName(parser);

  if (!expect(parser, scanner, TOKEN_LEFT_PAREN)) {
    error(parser, "expected opening paren");
    return (FunctionStatement){.token = {.type = TOKEN_NULL}};
  }
  
  if (parser->current.type != TOKEN_RIGHT_PAREN) {
    advance(parser, scanner);
    fs.args[fs.argCount++] = createName(parser);

    while (parser->current.type == TOKEN_COMMA) {
      advance(parser, scanner);
      advance(parser, scanner);
      fs.args[fs.argCount++] = createName(parser);
    }
  }

  // consume closing paren
  advance(parser, scanner);

  if (!expect(parser, scanner, TOKEN_LEFT_BRACE)) {
    error(parser, "expected opening brace");
    return (FunctionStatement){.token = {.type = TOKEN_NULL}};
  }

  fs.block = parseBlockStatement(parser, scanner);

  return fs;
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
    case TOKEN_LEFT_BRACE: {
      // parse block statement
      BlockStatement bs = parseBlockStatement(parser, scanner);
      stmt.type = STMT_BLOCK;
      stmt.data.blockStmt = bs;
      break;
    }
    case TOKEN_IF: {
      IfStatement is = parseIfStatement(parser, scanner);
      stmt.type = STMT_IF;
      stmt.data.ifStmt = is;
      break;
    }
    case TOKEN_WHILE: {
      WhileStatement ws = parseWhileStatement(parser, scanner);
      stmt.type = STMT_WHILE;
      stmt.data.whileStmt = ws;
      break;
    }
    case TOKEN_IDENTIFIER: {
      if (parser->current.type == TOKEN_LEFT_PAREN) {
        ExpressionStatement es = parseExpressionStatement(parser, scanner);
        stmt.type = STMT_EXPR;
        stmt.data.expressionStmt = es;
        break;
      }

      AssignStatement as = parseAssignStatement(parser, scanner);
      stmt.type = STMT_ASSIGN;
      stmt.data.assignStmt = as;
      break;
    }
    case TOKEN_FUNCTION: {
      FunctionStatement fs = parseFunctionStatement(parser, scanner);
      stmt.type = STMT_FUNCTION;
      stmt.data.funcStmt = fs;
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

static void freeCall(CallExpression* call) {
  for (int i = 0; i < call->argCount; i++) {
    freeExpression(call->args + i);
  }
  FREE_ARRAY(Expression, call->args, call->capacity);
  call->args = NULL;
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
    case EXPR_CALL:
      freeCall(&expr->data.call);
      break;
    default:
      return;
  }
}
