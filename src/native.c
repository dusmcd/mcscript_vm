#include "value.h"
#include <memory.h>
#include <native.h>
#include <table.h>
#include <object.h>
#include <string.h>
#include <stdio.h>

/*
 * native C functions
 *
 */
static Value print(VM* vm, int numArgs, Value* args) {
  for (int i = 0; i < numArgs; i++) {
    printValue(args[i]);
    printf(" ");
  }
  printf("\n");

  return NULL_VAL;
}

static Value writeTextToFile(VM* vm, int numArgs, Value* args) {
  if (numArgs != 2) {
    fprintf(stderr, "ERROR: expecting two arguments\n");
    return NULL_VAL;
  }

  Value fileNameVal = args[0];
  if (!(IS_OBJ(fileNameVal)) || !(IS_STRING(fileNameVal))) {
    fprintf(stderr, "ERROR: expecting a string for the file name\n");
    return NULL_VAL;
  }

  const char* fileName = AS_CSTRING(fileNameVal);
  FILE* file = fopen(fileName, "w");
  if (file == NULL) {
    fprintf(stderr, "ERROR: could not open file\n");
    return NULL_VAL;
  }

  Value dataVal = args[1];
  if (!(IS_OBJ(dataVal)) || !(IS_STRING(dataVal))) {
    fprintf(stderr, "ERROR: expecting a string for data\n");
    return NULL_VAL;
  }

  const char* data = AS_CSTRING(dataVal);
  fprintf(file, "%s\n", data);

  if (fclose(file) == -1) {
    fprintf(stderr, "ERROR: error closing file\n");
    return NULL_VAL;
  }
  return BOOL_VAL(true);
}

static Value readFile(VM* vm, int numArgs, Value* args) {
  if (numArgs != 1 || !(IS_OBJ(args[0])) || !(IS_STRING(args[0]))) {
    fprintf(stderr, "ERROR: must pass one string for file name\n");
    return NULL_VAL;
  }

  const char* fileName = AS_CSTRING(args[0]);
  FILE* file = fopen(fileName, "r");
  if (file == NULL) {
    fprintf(stderr, "ERROR: could not open file\n");
    return NULL_VAL;
  }

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char* str = ALLOCATE(char, size + 1);

  int bytesRead = fread(str, 1, size, file);
  if (bytesRead != size) {
    fprintf(stderr, "ERROR: error reading file\n");
    free(str);
    return NULL_VAL;
  }
  str[size] = '\0';

  if (fclose(file) == -1) {
    fprintf(stderr, "ERROR: error closing file\n");
    free(str);
    return NULL_VAL;
  }

  ObjString* obj = allocateString(vm, str);
  return OBJ_VAL(obj);
}

/*
 * helper functions
 *
 */

static ObjString* createKey(VM* vm, const char* str) {
  int length = strlen(str);
  char* keyStr = ALLOCATE(char, length + 1);
  strcpy(keyStr, str);
  keyStr[length] = '\0';
  ObjString* key = allocateString(vm, keyStr);

  return key;
}

void defineNatives(VM* vm) {
  ObjNative* native = newNative(vm, print);
  ObjString* key = createKey(vm, "print");
  tableSet(&vm->globals, key, OBJ_VAL(native));

  native = newNative(vm, readFile);
  key = createKey(vm, "readFile");
  tableSet(&vm->globals, key, OBJ_VAL(native));

  native = newNative(vm, writeTextToFile);
  key = createKey(vm, "readTextToFile");
  tableSet(&vm->globals, key, OBJ_VAL(native));
}
