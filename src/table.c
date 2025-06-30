#include <value.h>
#include <table.h>
#include <memory.h>
#include <stdlib.h>
#include <stdint.h>
#include <object.h>
#include <string.h>

#define TABLE_MAX_LOAD 0.75

void initTable(Table *table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void freeTable(Table* table) {
  FREE_ARRAY(Table, table->entries, table->capacity);
  initTable(table);
}

void tableAddAll(Table* to, Table* from) {
  for (int i = 0; i < from->capacity; i++) {
    Entry* entry = &from->entries[i];
    if (entry->key != NULL) {
      tableSet(to, entry->key, entry->value);
    }
  }
}

static inline bool keysEqual(const ObjString* a, const ObjString* b) {
  return strcmp(a->str, b->str) == 0;
}



/**
 * find the entry with the given key if it exists
 * if it doesn't, then return address of new entry
 * to use
 */
static Entry* findEntry(Entry* entries, ObjString* key, int capacity) {
  uint32_t index = key->hash % capacity;

  /**
   * check for collisions and increase index linearly
   * if there are collisions
   */
  while(true) {
    Entry* entry = &entries[index];
    if (entry->key == NULL || keysEqual(entry->key, key)) {
      return entry;
    }
    index = (index + 1) % capacity;
  }
}

bool tableGet(Table* table, ObjString* key, Value* value) {
  if (table->count == 0) return false;

  Entry* entry = findEntry(table->entries, key, table->capacity);
  if (entry->key == NULL) return false;

  *value = entry->value;

  return true;
}


/**
 * create an array of entries will null values
 */
static void adjustCapacity(Table* table) {
  Entry* entries = ALLOCATE(Entry, table->capacity);
  for (int i = 0; i < table->capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = NULL_VAL;
  }

  /**
   * copying over old table to new table with
   * increased capacity
   */
  if (table->count > 0) {
    for (int i = 0; i < table->capacity; i++) {
      Entry* entry = &table->entries[i];
      if (entry->key == NULL) continue;

      /**
       * this retrieves the pointer to the new entry in the
       * new table. This will always return a null entry
       */
      Entry* dest = findEntry(entries, entry->key, table->capacity);

      dest->key = entry->key;
      dest->value = entry->value;
    }

    FREE_ARRAY(Entry, table->entries, 0);
  }
  
  table->entries = entries;
}


bool tableSet(Table* table, ObjString* key, Value value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    table->capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table);
  }

  Entry* entry = findEntry(table->entries, key, table->capacity);
  bool isNewKey = entry->key == NULL;

  if (isNewKey) {
    table->count++;
  }
  entry->key = key;
  entry->value = value;


  return isNewKey;
}
