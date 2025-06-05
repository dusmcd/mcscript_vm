#include <parser.h>
#include <stdbool.h>
#include <stdio.h>
#include <memory.h>

static void advance(Parser* parser, Scanner* scanner) {
  parser->previous = parser->current;
  parser->current = scanToken(scanner);
}

static void initParser(Parser* parser, Scanner* scanner) {
  advance(parser, scanner);
  advance(parser, scanner);
}

static ReturnStatement parseReturnStatement(Parser* parser) {
  ReturnStatement rs;
  return rs;
}

static Statement parseStatement(Parser* parser) {
  Statement stmt;
  switch(parser->previous.type) {
    case TOKEN_RETURN: {
      ReturnStatement rs = parseReturnStatement(parser);
      stmt.type = STMT_RETURN;
      stmt.data.returnStmt = rs;
      return stmt;
    }
    default:
     return stmt;
  }
}

static void append(Statements* statements, Statement stmt) {
  int oldCapacity = statements->capacity;
  statements->capacity = GROW_CAPACITY(statements->capacity);
  statements->stmts = GROW_ARRAY(Statement, statements->stmts, oldCapacity, statements->capacity);
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
    printf("[line: %d]{ type: %d, literal: %.*s }\n", 
        parser->previous.line, parser->previous.type,
        parser->previous.length, parser->previous.start);

    Statement stmt = parseStatement(parser);
    append(&statements, stmt);    

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
