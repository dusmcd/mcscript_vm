#ifndef MCSCRIPT_VM_OBJECT_H
#define MCSCRIPT_VM_OBJECT_H

#include <value.h>
#include <stdbool.h>
#include <ast.h>
#include <vm.h>
#include <chunk.h>

#define OBJ_TYPE(value) AS_OBJ(value)->type
#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_FUNC(value) isObjType(value, OBJ_FUNCTION)

#define AS_STRING(value) (ObjString*)AS_OBJ(value)
#define AS_CSTRING(value) ((ObjString*)AS_OBJ(value))->str
#define AS_FUNC(value) (ObjFunction*)AS_OBJ(value)
#define CHUNK(func) func.chunk


/**
 * the set of all object types
 */
typedef enum {
  OBJ_STRING,
  OBJ_FUNCTION
} ObjType;

/**
 * container for data stored on the heap
 */
struct obj {
  ObjType type;
  Obj* next;
};

/**
 * string data type
 * "inherits" from the Obj type by making it the first
 * field in the struct
 */
struct objString {
  Obj obj;
  int length;
  char* str;
  uint32_t hash;
};

/**
 * function data type
 */
 struct ObjFunction {
  Obj obj;
  ObjString* name;
  int numArgs;
  Chunk chunk;
};

static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

/**
 * wrap strings as objects on the heap and track in vm
 */
ObjString* allocateString(VM* vm, char* str);

/**
 * create a string from an AST Expression
 * allocates char array on heap
 * don't forget to free it if passing to allocateString and
 * not using again
 */
char* createString(const Expression* expr);

/**
 * hash function for a string
 */
uint32_t hashString(const char* key, int length);

ObjFunction* newFunction(VM* vm);

#endif
