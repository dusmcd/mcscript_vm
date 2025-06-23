#ifndef MCSCRIPT_VM_MEMORY_H
#define MCSCRIPT_VM_MEMORY_H

#include <stdlib.h>

/**
 * macros for managing memory of dynamic arrays (e.g., Chunk, ValueArray)
 * and other dynamically allocated objects
 */

#define GROW_CAPACITY(capacity) \
  (capacity) == 0 ? 8 : (capacity) * 2

#define GROW_ARRAY(type, pointer, oldCapacity, newCapacity) \
  (type*)reallocate(pointer, sizeof(type) * oldCapacity, sizeof(type) * newCapacity)

#define FREE_ARRAY(type, pointer, oldCapacity) \
  (type*)reallocate(pointer, sizeof(type) * oldCapacity, 0)

#define ALLOCATE(type, size) \
  (type*)reallocate(NULL, 0, sizeof(type) * size)

#define FREE(type, pointer) \
  reallocate(pointer, sizeof(type), 0)

/**
 * allocates memory on the heap
 * will resize as needed
 */
void* reallocate(void* pointer, size_t oldCapacity, size_t newCapacity);

#endif
