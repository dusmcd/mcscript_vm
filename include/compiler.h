#ifndef MCSCRIPT_VM_COMPILER_H
#define MCSCRIPT_VM_COMPILER_H

#include <chunk.h>
#include <scanner.h>
#include <stdbool.h>
#include <parser.h>

/**
 * compile the source code into bytecode instructions
 * and store them in chunk
 */
bool compile(Chunk* chunk, Statements statements);

#endif
