#ifndef MCSCRIPT_VM_COMPILER_H
#define MCSCRIPT_VM_COMPILER_H

#include <chunk.h>

/**
 * compile the source code into bytecode instructions
 * and store them in chunk
 */
void compile(Chunk* chunk, const char* source);

#endif
