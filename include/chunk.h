#ifndef MCSCRIPT_VM_CHUNK_H
#define MCSCRIPT_VM_CHUNK_H

#include <stdint.h>
#include <value.h>

/**
 * the set of all bytecode instructions
 */
typedef enum {
  /**
   * represents type Value
   * will be followed with parameters that are the index into the ValueArray
   */
  OP_CONSTANT,
  OP_NEGATE,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_TRUE,
  OP_FALSE,
  OP_NOT,
  OP_LESS,
  OP_GREATER,
  OP_EQUAL,
  OP_DEFINE_GLOBAL,
  OP_NULL,
  OP_RETURN // return instruction (i.e., pop function off stack and return to next instruction)
} OpCode;


/**
 * represents a dynamic array of OpCodes (see enum in "common.h")
 * the constants array is for literals in the source code
 */
typedef struct {
  int capacity;
  int count;
  uint8_t* code;
  int* lines;
  ValueArray constants;
} Chunk;

/**
 * initialize an empty chunk
 */
void initChunk(Chunk* chunk);

/**
 * add a single op code (one byte long) to chunk
 * also add line number
 */
void writeChunk(Chunk* chunk, uint8_t byte, int line);

/**
 * add a value to the constants array 
 * returns the index where data is stored
 */
int addConstant(Chunk* chunk, Value val);

/**
 * free up dynamic arrays store on the heap
 */
void freeChunk(Chunk* chunk);

#endif
