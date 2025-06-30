#ifndef MCSCRIPT_VM_OBJECT_H
#define MCSCRIPT_VM_OBJECT_H

#include <value.h>
#include <stdbool.h>
#include <ast.h>
#include <vm.h>

#define OBJ_TYPE(value) AS_OBJ(value)->type
#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define AS_STRING(value) (ObjString*)AS_OBJ(value)
#define AS_CSTRING(value) ((ObjString*)AS_OBJ(value))->str


/**
 * the set of all object types
 */
typedef enum {
  OBJ_STRING
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

static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

/**
 * wrap strings as objects on the heap and track in vm
 */
ObjString* allocateString(VM* vm, const char* str);

/**
 * create a string from an AST Expression
 * allocates char array on heap
 * don't forget to free it if passing to allocateString and
 * not using again
 */
const char* createString(const Expression* expr);

/**
 * hash function for a string
 */
uint32_t hashString(const char* key, int length);

#endif
