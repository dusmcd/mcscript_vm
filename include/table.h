#ifndef MCSCRIPT_VM_TABLE_H
#define MCSCRIPT_VM_TABLE_H

#include <value.h>

typedef struct {
  ObjString* key;
  Value value;
} Entry;

typedef struct {
  int count;
  int capacity;
  Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(Table* table);

/**
 * insert a new key/value pair into table
 */
bool tableSet(Table* table, ObjString* key, Value value);

/**
 * get a value associated with a key
 * the value will be inserted to value passed in
 */
bool tableGet(Table* table, ObjString* key, Value* value);

bool tableDelete(Table* table, ObjString* key);

/**
 * copy all the entries in "from" to "to"
 */
void tableAddAll(Table* to, Table* from);




#endif
