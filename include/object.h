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
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)

#define AS_STRING(value) (ObjString*)AS_OBJ(value)
#define AS_CSTRING(value) ((ObjString*)AS_OBJ(value))->str
#define AS_FUNC(value) (ObjFunction*)AS_OBJ(value)
#define AS_NATIVE(value) (ObjNative*)AS_OBJ(value)
#define CHUNK(func) func.chunk


/**
 * the set of all object types
 */
typedef enum {
  OBJ_STRING,
  OBJ_FUNCTION,
  OBJ_NATIVE
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

/**
 * function pointer to native C functions
 * I.e., clock, read file, write file, etc.
 */
typedef Value (*NativeFunc)(VM* vm, int numArgs, Value* args);

/**
 * object to wrap native C functions
 */
typedef struct {
  Obj obj;
  NativeFunc func;
} ObjNative;

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
 */
char* createString(const Expression* expr);

/**
 * hash function for a string
 */
uint32_t hashString(const char* key, int length);

ObjFunction* newFunction(VM* vm);
ObjNative* newNative(VM* vm, NativeFunc func);

#endif
