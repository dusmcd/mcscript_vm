#ifndef MCSCRIPT_VM_MEMORY_H
#define MCSCRIPT_VM_MEMORY_H

#include <stdlib.h>

#define GROW_CAPACITY(capacity) \
  (capacity) == 0 ? 8 : (capacity) * 2

#define GROW_ARRAY(type, pointer, oldCapacity, newCapacity) \
  (type*)reallocate(pointer, sizeof(type) * oldCapacity, sizeof(type) * newCapacity)

#define FREE_ARRAY(type, pointer, oldCapacity) \
  (type*)reallocate(pointer, sizeof(type) * oldCapacity, 0)

void* reallocate(void* pointer, size_t oldCapacity, size_t newCapacity);

#endif
