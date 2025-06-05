#ifndef MCSCRIPT_VM_COMPILER_H
#define MCSCRIPT_VM_COMPILER_H

#include <chunk.h>
#include <scanner.h>
#include <stdbool.h>
#include <parser.h>

/**
 * precedence hierarchy for different operators
 */
typedef enum {
  PREC_NONE,
  PREC_ASSIGN,
  PREC_TERM,
  PREC_FACTOR,
  PREC_UNARY,
  PREC_CALL
} Precedence;

/**
 * compile the source code into bytecode instructions
 * and store them in chunk
 */
bool compile(Chunk* chunk, Statements statements);

#endif
