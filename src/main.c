#include <chunk.h>
#include <stdio.h>
#include <common.h>
#include <debug.h>
#include <vm.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

static const char* readFile(const char* fileName) {
  FILE* file = fopen(fileName, "r");
  if (file == NULL) {
    fprintf(stderr, "Could not open file\n");
    exit(60);
  }

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  if (size == 0) {
    exit(50);
  }

  char* source = (char*)malloc(size + 1);
  if (source == NULL) {
    fprintf(stderr, "insufficient memory.\n");
    exit(40);
  }

  size_t bytesRead = fread(source, 1, size, file);

  if (bytesRead != size) {
    fprintf(stderr, "error reading file\n");
    fclose(file);
    free(source);
    exit(30);
  }

  source[size] = '\0';

  if (fclose(file) == -1) {
    fprintf(stderr, "error closing file\n");
    exit(20);
  }

  return source;
}

static void repl(VM* vm) {
  printf("Welcome to VMScript v. 0.1 Programming language\n");
  printf("Begin typing commands. Type 'exit' to terminate\n");

  while(true) {
    printf(">> ");
    char line[1024];

    fgets(line, sizeof(line), stdin);

    if (memcmp(line, "exit", 4) == 0) {
      break;
    }

    InterpretResult result = interpret(vm, line);
    freeChunk(vm->chunk);
    resetVM(vm);
    if (result == COMPILE_ERROR) {
      fprintf(stderr, "compilation error\n");
    }

    if (result == RUNTIME_ERROR) {
      fprintf(stderr, "runtime error\n");
    }
  }
}


int main(int argc, char** argv) {
  Chunk chunk;
  initChunk(&chunk);

  VM vm;
  initVM(&vm, &chunk);

  if (argc == 1) {
    // run repl
    repl(&vm);
  } else if (argc == 2) {
    const char* source = readFile(argv[1]);
    InterpretResult result = interpret(&vm, source);
    free((char*)source);
    source = NULL;
    freeChunk(&chunk);

    if (result == COMPILE_ERROR) {
      fprintf(stderr, "compilation error\n");
      exit(70);
    }

    if (result == RUNTIME_ERROR) {
      fprintf(stderr, "runtime error\n");
      exit(80);
    }
  } else {
    fprintf(stderr, "usage: mcscript_vm <path | optional>\n");
    return -1;
  }
  
  return 0;
}
