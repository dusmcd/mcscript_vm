#include <value.h>
#include <table_test.h>
#include <table.h>
#include <stdio.h>
#include <object.h>
#include <string.h>

static ObjString createKey(char* str) {
  int length = strlen(str);
  uint32_t hash = hashString(str, length);
  ObjString key = {
    .obj = {.type = OBJ_STRING},
    .str = str,
    .length = length,
    .hash = hash
  };

  return key;
}

static void testSetTable() {
  TableTest test = {
    .count = 2,
    .keys = {"height", "age"},
    .vals = {NUMBER_VAL(60), NUMBER_VAL(36)}
  };

  Table table;
  initTable(&table);

  for (int i = 0; i < test.count; i++) {
    char* str = (char*)test.keys[i];
    ObjString key = createKey(str);
    
    bool newKey = tableSet(&table, &key, test.vals[i]);
    if (!newKey) {
      fprintf(stderr, "%s key already set\n", str);
      return;
    }

    newKey = tableSet(&table, &key, NUMBER_VAL(100));
    if (newKey) {
      fprintf(stderr, "%s key not set\n", str);
      return;
    }

  }

  freeTable(&table);
  puts("testSetTable() passed");
}

static void testGetTable() {
  TableTest test = {
    .count = 2,
    .keys = {"height", "age"},
    .vals = {NUMBER_VAL(60), NUMBER_VAL(36)}
  };

  Table table;
  initTable(&table);

  for (int i = 0; i < test.count; i++) {
    char* str = (char*)test.keys[i];
    ObjString key = createKey(str);
    Value val = test.vals[i];
    tableSet(&table, &key, val);

    Value emptyVal;
    bool found = tableGet(&table, &key, &emptyVal);

    if (!found) {
      fprintf(stderr, "%s key not found\n", str);
      return;
    }

    if (!(IS_NUM(emptyVal))) {
      fprintf(stderr, "found value is not number\n");
      return;
    }

    if (AS_NUMBER(val) != AS_NUMBER(emptyVal)) {
      fprintf(stderr, "wrong value expected=%f got=%f\n",
          AS_NUMBER(val), AS_NUMBER(emptyVal));
      return;
    }
    
  }
  freeTable(&table);
  puts("testGetTable() passed");
}

static void testDeleteTable() {
  TableTest test = {
    .count = 2,
    .keys = {"height", "age"},
    .vals = {NUMBER_VAL(60), NUMBER_VAL(36)}
  };
  
  Table table;
  initTable(&table);

  for (int i = 0; i < test.count; i++) {
    char* str = (char*)test.keys[i];
    ObjString key = createKey(str);
    Value val = test.vals[i];
    tableSet(&table, &key, val);

    bool success = tableDelete(&table, &key);
    if (!success) {
      fprintf(stderr, "%s was not sucessfully deleted\n", str);
      return;
    }
    Value emptyVal;
    bool found = tableGet(&table, &key, &emptyVal);

    if (found) {
      fprintf(stderr,"%s key was found after deletion\n", str);
      return;
    }
  }
  freeTable(&table);
  puts("testDeleteTable() passed");

}

void testTable() {
  printf("=== Hash Table Tests ===\n");
  testSetTable();
  testGetTable();
  testDeleteTable();
}
