#include <scanner.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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
        return;
      default:
        return;
    }
  }
}

static Token errorToken(Scanner* scanner, const char* message) {
  Token token = {
    .length = (int)(scanner->current - scanner->start),
    .start = scanner->start,
    .type = TOKEN_ILLEGAL,
    .line = scanner->line
  };

  fprintf(stderr,"%s\n", message);

  return token;
}

static Token string(Scanner* scanner) {
  while (peek(scanner) != '"') {
    if (isAtEnd(scanner)) {
      return errorToken(scanner, "Unterminated string literal.");
    }
    advance(scanner);
  }

  // consume closing quotation
  advance(scanner);

  Token token = {
    .length = (int)(scanner->current - scanner->start),
    .line = scanner->line,
    .start = scanner->start,
    .type = TOKEN_STRING
  };

  return token;
}

static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') ||
    (c >= 'A' && c <= 'Z') ||
    c == '_';
}

static TokenType getIdentifierType(Scanner* scanner, int length) {
  switch(scanner->start[0]) {
    case 'a': 
      // looking for "and"
      if (length == 3 &&
          memcmp(scanner->start + 1, "nd", 2) == 0) {
        return TOKEN_AND;
      }
      break;
    case 'e':
      if (length == 4 &&
          memcmp(scanner->start + 1, "lse", 3) == 0) {
        return TOKEN_ELSE;
      }
      break;
    case 'f':
      if (length == 3 &&
          memcmp(scanner->start + 1, "or", 2) == 0) {
        return TOKEN_FOR;
      } else if (length == 5 &&
         memcmp(scanner->start + 1, "alse", 4) == 0) {
        return TOKEN_FALSE;
      } 
      else if (length == 8 &&
          memcmp(scanner->start + 1, "unction", 7) == 0) {
        return TOKEN_FUNCTION;
      }
      break;
    case 'o':
      if (length == 2 &&
          memcmp(scanner->start + 1, "r", 1) == 0) {
        return TOKEN_OR;
      }
      break;
    case 'p':
      if (length == 5 &&
          memcmp(scanner->start + 1, "rint", 4) == 0) {
        return TOKEN_PRINT;
      }
      break;
    case 'r':
      if (length == 6 &&
          memcmp(scanner->start + 1, "eturn",5) == 0) {
        return TOKEN_RETURN;
      }
      break;
    case 't':
      if (length == 4 &&
          memcmp(scanner->start + 1, "rue", 3) == 0) {
        return TOKEN_TRUE;
      }
    case 'v':
      if (length == 3 &&
          memcmp(scanner->start + 1, "ar", 2) == 0) {
        return TOKEN_VAR;
      }
      break;
    case 'w':
      if (length == 5 &&
          memcmp(scanner->start + 1, "hile", 4) == 0) {
        return TOKEN_WHILE;
      }
      break;
    }

  return TOKEN_IDENTIFIER;
}

static Token getIdentifier(Scanner* scanner) {
  while(isAlpha(peek(scanner))) {
    advance(scanner);
  }

  Token token = {
    .length = (int)(scanner->current - scanner->start),
    .line = scanner->line,
    .start = scanner->start
  };

  token.type = getIdentifierType(scanner, token.length);

  return token;
}


Token scanToken(Scanner* scanner) {
  skipWhiteSpace(scanner);
  scanner->start = scanner->current;
  char c = advance(scanner);
  if (isDigit(c)) return number(scanner);
  if (isAlpha(c)) return getIdentifier(scanner);

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
    case ',': return makeToken(scanner, TOKEN_COMMA);
    case '"': return string(scanner);
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
    default: return errorToken(scanner, "Unexpected token.");
  }

}
