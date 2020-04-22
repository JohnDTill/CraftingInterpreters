#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"

typedef struct {
    HeapObjString* key;
    Value value;
} TableEntry;

typedef struct {
    unsigned count;
    unsigned capacity;
    TableEntry* entries;
} HashTable;

void initTable(HashTable* table);
void freeTable(HashTable* table);
bool tableGet(HashTable* table, HeapObjString* key, Value* value);
bool tableSet(HashTable* table, HeapObjString* key, Value value);
bool tableDelete(HashTable* table, HeapObjString* key);
void tableAddAll(HashTable* from, HashTable* to);
HeapObjString* tableFindString(HashTable* table, const char* chars, unsigned length, uint32_t hash);

#endif
