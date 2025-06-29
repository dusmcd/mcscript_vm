#include <value.h>
#include <table.h>
#include <memory.h>
#include <stdlib.h>
#include <stdint.h>
#include <object.h>

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
    if (entry->key == key || entry->key == NULL) {
      return entry;
    }
    index = (index + 1) % capacity;
  }
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

  table->entries = entries;
}


bool tableSet(Table* table, ObjString* key, Value value) {
  if (table->count + 1 < table->capacity * TABLE_MAX_LOAD) {
    table->capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table);
  }

  Entry* entry = findEntry(table->entries, key, table->capacity);
  bool isNewKey = entry->key == NULL;

  if (isNewKey) {
    entry->key = key;
    entry->value = value;
  }

  return isNewKey;
}
