#include "chunk.h"
#include <object.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

uint32_t hashString(const char* key, int length) {
  // this is a hash function
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }

  return hash;
}

static void trackObject(VM* vm, Obj* obj) {
  obj->next = vm->objects;
  vm->objects = obj;
}

ObjString* allocateString(VM* vm, char* buff) {
  ObjString* obj = ALLOCATE(ObjString, 1);
  if (obj == NULL) {
    return NULL;
  }
  
  int length = strlen(buff);

  obj->obj.type = OBJ_STRING;
  obj->length = length;
  obj->str = buff;
  obj->hash = hashString(buff, obj->length);

  trackObject(vm, (Obj*)obj);

  return obj;
}



char* createString(const Expression* expr) {
  if (expr->type != EXPR_STRING) {
    return NULL;
  }

  String stringExpr = expr->data.string;

  // not including quotation marks in value
  int length = stringExpr.token.length - 2;
  char* chars = ALLOCATE(char, length + 1);
  if (chars == NULL) {
    return NULL;
  }

  memcpy(chars, stringExpr.token.start + 1, length);
  chars[length] = '\0';

  return chars;
}

ObjFunction* newFunction(VM* vm) {
  ObjFunction* func = ALLOCATE(ObjFunction, 1);

  if (func == NULL) return NULL;

  func->name = NULL;
  func->numArgs = 0;
  initChunk(&func->chunk);

  trackObject(vm, (Obj*)func);

  return func;
}
