#ifndef MCSCRIPT_VM_COMPILER_H
#define MCSCRIPT_VM_COMPILER_H

#include <chunk.h>
#include <scanner.h>
#include <stdbool.h>
#include <parser.h>
#include <stdint.h>
#include <vm.h>
#include <object.h>

#define UINT8_COUNT (UINT8_MAX + 1)

/**
 * a local variable
 */
typedef struct {
  Token name;
  int depth;
} Local;

typedef enum {
  TYPE_SCRIPT,
  TYPE_FUNCTION
} FunctionType;

/**
 * data structure to keep track of local variables
 */
struct Compiler {
  Local locals[UINT8_COUNT];
  int localCount;
  int scopeDepth;
  ObjFunction* func;
  FunctionType type;
};

typedef struct {
  bool hasError;
  ObjFunction* func;
} CompilerResult;

/**
 * compile the source code into bytecode instructions
 * and store them in chunk
 */
CompilerResult compile(VM* vm, const Statements* statements);
void initCompiler(VM* vm, Compiler* compiler, FunctionType type);

#endif
