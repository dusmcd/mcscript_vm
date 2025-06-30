#include <value.h>
#include <table_test.h>
#include <table.h>
#include <stdio.h>
#include <object.h>
#include <string.h>

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
    int length = strlen(str);
    uint32_t hash = hashString(str, length);
    ObjString key = {
      .obj = {.type = OBJ_STRING},
      .str = str,
      .length = length,
      .hash = hash
    };

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

void testTable() {
  printf("=== Hash Table Tests ===\n");
  testSetTable();
}
