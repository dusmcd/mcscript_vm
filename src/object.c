#include <object.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

static uint32_t hashString(const char* key, int length) {
  // this is a hash function
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }

  return hash;
}

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
  str->hash = hashString(chars, str->length);

  return (Obj*)str;
}


Obj* createObject(VM* vm, const Expression* expr, ObjType type) {
  // when the garbage collector is implemented this is where
  // the object tracking would start
  Obj* obj;
  
  switch(type) {
    case OBJ_STRING: {
      obj = createString(expr);
      break;
    }
    default:
      return NULL;
  }

  obj->next = vm->objects;
  vm->objects = obj;

  return obj;
}
