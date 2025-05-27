#ifndef MCSCRIPT_VM_DEBUG_H
#define MCSCRIPT_VM_DEBUG_H

#include <stdint.h>
#include <chunk.h>

/**
 * functions for converting bytecode instructions into human-readable output
 */
void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif
