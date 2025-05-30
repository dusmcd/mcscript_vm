#include <scanner.h>
#include <stdlib.h>
#include <stdbool.h>

void initScanner(Scanner* scanner, const char* source) {
  scanner->start = source;
  scanner->current = source;
  scanner->line = 1;
}

void freeScanner(Scanner* scanner) {
}

static char advance(Scanner* scanner) {
  scanner->current++;
  return scanner->current[-1];
}

static Token makeToken(Scanner* scanner, TokenType type) {
  Token token = {
    .type = type,
    .length = (int)(scanner->current - scanner->start),
    .line = scanner->line,
    .start = scanner->start
  };

  return token;
}

static bool isAtEnd(Scanner* scanner) {
  return *(scanner->current) == '\0';
}

static char peek(Scanner* scanner) {
  return *(scanner->current);
}

static bool match(Scanner* scanner, char c) {
  if (isAtEnd(scanner)) return false;

  if (peek(scanner) == c) {
    advance(scanner);
    return true;
  }

  return false;

}

static bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

static char peekNext(Scanner* scanner) {
  return scanner->current[1];
}

static Token number(Scanner* scanner) {
  while (!isAtEnd(scanner) && isDigit(peek(scanner))) {
    advance(scanner);
  }

  if (peek(scanner) == '.' && isDigit(peekNext(scanner))) {
    // consume '.'
    advance(scanner);
    while (isDigit(peek(scanner))) {
      advance(scanner);
    }
  }

  return makeToken(scanner, TOKEN_NUMBER);
}

static void skipWhiteSpace(Scanner* scanner) {
  while (true) {
    switch(peek(scanner)) {
      case ' ':
      case '\t':
      case '\r':
        advance(scanner);
        break;
      case '\n':
        // increment the line number when we see a line break
        scanner->line++;
        advance(scanner);
        break;
      case '/':
        // comments start with '//' so we will advance until we reach a new line
        if (peekNext(scanner) == '/') {
          while(peek(scanner) != '\n') advance(scanner);
        }
        break;
      default:
        return;
    }
  }
}


Token scanToken(Scanner* scanner) {
  skipWhiteSpace(scanner);
  scanner->start = scanner->current;
  char c = advance(scanner);
  if (isDigit(c)) return number(scanner);

  switch(c) {
    case '(': return makeToken(scanner, TOKEN_LEFT_PAREN);
    case ')': return makeToken(scanner, TOKEN_RIGHT_PAREN);
    case '{': return makeToken(scanner, TOKEN_LEFT_BRACE);
    case '}': return makeToken(scanner, TOKEN_RIGHT_BRACE);
    case '+': return makeToken(scanner, TOKEN_PLUS);
    case '-': return makeToken(scanner, TOKEN_MINUS);
    case '*': return makeToken(scanner, TOKEN_STAR);
    case ';': return makeToken(scanner, TOKEN_SEMICOLON);
    case '/': return makeToken(scanner, TOKEN_SLASH);
    case '!': 
      return match(scanner, '=') ? makeToken(scanner, TOKEN_BANG_EQUAL)
        : makeToken(scanner, TOKEN_BANG);
    case '<':
      return match(scanner, '=') ? makeToken(scanner, TOKEN_LESS_EQUAL)
        : makeToken(scanner, TOKEN_LESS);
    case '>':
      return match(scanner, '=') ? makeToken(scanner, TOKEN_GREATER_EQUAL)
        : makeToken(scanner, TOKEN_GREATER);
    case '=':
      return match(scanner, '=') ? makeToken(scanner, TOKEN_EQUAL_EQUAL)
        : makeToken(scanner, TOKEN_EQUAL);
    case '\0': return makeToken(scanner, TOKEN_EOF);
    default: return makeToken(scanner, TOKEN_ILLEGAL);
  }

}
