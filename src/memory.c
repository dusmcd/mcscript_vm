#include <memory.h>

#include <stdlib.h>

void* reallocate(void* pointer, size_t oldCapacity, size_t newCapacity) {
  if (newCapacity == 0) {
    free(pointer);
    return NULL;
  }

  void* result = realloc(pointer, newCapacity);
  if (result == NULL) exit(1);
  return result;
}
