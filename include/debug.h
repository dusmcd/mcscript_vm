#ifndef MCSCRIPT_VM_DEBUG_H
#define MCSCRIPT_VM_DEBUG_H

#include <stdint.h>
#include <chunk.h>

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif
