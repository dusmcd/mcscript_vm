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
static Value print(int numArgs, Value* args) {
  for (int i = 0; i < numArgs; i++) {
    printValue(args[i]);
    printf(" ");
  }
  printf("\n");

  return (Value){0};
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
}
