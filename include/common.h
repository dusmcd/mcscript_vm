#ifndef MSCRIPT_VM_COMMON_H
#define MSCRIPT_VM_COMMON_H

/**
 * the set of all bytecode instructions
 */
typedef enum {
  /**
   * represents type Value
   * will be followed with parameters that are the index into the ValueArray
   */
  OP_CONSTANT,
  OP_RETURN // return instruction (i.e., pop function off stack and return to next instruction)
} OpCode;

#endif
