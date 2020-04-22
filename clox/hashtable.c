#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "hashtable.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(HashTable* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(HashTable* table) {
    FREE_ARRAY(TableEntry, table->entries, table->capacity);
    initTable(table);
}

static TableEntry* findEntry(TableEntry* entries, unsigned capacity, HeapObjString* key) {
    uint32_t index = key->hash % capacity;
    TableEntry* tombstone = NULL;

    for (;;) {
        TableEntry* entry = &entries[index];

        if (entry->key == NULL) {
            if (VALUE_TYPE_IS_NIL(entry->value)) {
                // Empty entry.
                return tombstone != NULL ? tombstone : entry;
            } else {
                // We found a tombstone.
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key) {
            // We found the key.
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

bool tableGet(HashTable* table, HeapObjString* key, Value* value) {
    if (table->count == 0) return false;

    TableEntry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

static void adjustCapacity(HashTable* table, unsigned capacity) {
    TableEntry* entries = ALLOCATE(TableEntry, capacity);
    for (unsigned i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = CREATE_NIL_VAL;
    }

    table->count = 0;
    for (unsigned i = 0; i < table->capacity; i++) {
        TableEntry* entry = &table->entries[i];
        if (entry->key == NULL) continue;

        TableEntry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    FREE_ARRAY(TableEntry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool tableSet(HashTable* table, HeapObjString* key, Value value) {
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        unsigned capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    TableEntry* entry = findEntry(table->entries, table->capacity, key);

    bool isNewKey = entry->key == NULL;
    if (isNewKey && VALUE_TYPE_IS_NIL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableDelete(HashTable* table, HeapObjString* key) {
    if (table->count == 0) return false;

    // Find the entry.
    TableEntry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    // Place a tombstone in the entry.
    entry->key = NULL;
    entry->value = CREATE_BOOL_VAL(true);

    return true;
}

void tableAddAll(HashTable* from, HashTable* to) {
    for (unsigned i = 0; i < from->capacity; i++) {
        TableEntry* entry = &from->entries[i];
        if (entry->key != NULL) {
            tableSet(to, entry->key, entry->value);
        }
    }
}

HeapObjString* tableFindString(HashTable* table, const char* chars, unsigned length, uint32_t hash) {
  if (table->count == 0) return NULL;

  uint32_t index = hash % table->capacity;

  for (;;) {
    TableEntry* entry = &table->entries[index];

    if (entry->key == NULL) {
      // Stop if we find an empty non-tombstone entry.
      if (VALUE_TYPE_IS_NIL(entry->value)) return NULL;
    } else if (entry->key->length == length &&
        entry->key->hash == hash &&
        memcmp(entry->key->chars, chars, length) == 0) {
      // We found it.
      return entry->key;
    }

    index = (index + 1) % table->capacity;
  }
}
