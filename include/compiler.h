#ifndef MCSCRIPT_VM_COMPILER_H
#define MCSCRIPT_VM_COMPILER_H

#include <chunk.h>
#include <scanner.h>
#include <stdbool.h>
#include <parser.h>
#include <stdint.h>
#include <vm.h>

#define UINT8_COUNT (UINT8_MAX + 1)

/**
 * a local variable
 */
typedef struct {
  Token name;
  int depth;
} Local;

/**
 * data structure to keep track of local variables
 */
struct Compiler {
  Local locals[UINT8_COUNT];
  int localCount;
  int scopeDepth;
};

/**
 * compile the source code into bytecode instructions
 * and store them in chunk
 */
bool compile(VM* vm, const Statements* statements, Compiler* compiler);
void initCompiler(Compiler* compiler);

#endif
