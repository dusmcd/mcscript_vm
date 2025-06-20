#include <object.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

static Obj* createString(const Expression* expr) {
  if (expr->type != EXPR_STRING) {
    return NULL;
  }

  ObjString* str = ALLOCATE(ObjString, 1);

  if (str == NULL) {
    return NULL;
  }

  str->obj.type = OBJ_STRING;
  String stringExpr = expr->data.string;

  // not including quotation marks in value
  str->length = stringExpr.token.length - 2;
  char* chars = ALLOCATE(char, str->length + 1);
  if (chars == NULL) {
    return NULL;
  }

  memcpy(chars, stringExpr.token.start + 1, str->length);

  chars[str->length] = '\0';
  str->str = chars;

  return (Obj*)str;
}


Obj* createObject(const Expression* expr, ObjType type) {
  // when the garbage collector is implemented this is where
  // the object tracking would start
  
  switch(type) {
    case OBJ_STRING: {
      return createString(expr);
    }
    default:
      return NULL;
  }
}
