#include <chunk.h>
#include <stdio.h>
#include <common.h>
#include <debug.h>
#include <vm.h>


int main() {
  Chunk chunk;
  initChunk(&chunk);

  VM vm;
  initVM(&vm, &chunk);

  const char* source = "10 == 10; 10 != 11;\n"
                        "// I am a comment... ignore me\n"
                        "5 > 3; 5 >= 2;";
  interpret(&vm, source);
  
  freeChunk(&chunk);
  freeVM(&vm);

  return 0;
}
