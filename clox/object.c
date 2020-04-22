#include <stdio.h>
#include <string.h>

#include "hashtable.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

static HeapObj* allocateObject(size_t size, HeapObjType type) {
    HeapObj* object = (HeapObj*)reallocate(NULL, 0, size);
    object->type = type;

    object->next = vm.objects; //Store every allocated object for memory management
    vm.objects = object;
    return object;
}

static HeapObjString* allocateString(char* chars, unsigned length, uint32_t hash) {
    HeapObjString* string = ALLOCATE_OBJ(HeapObjString, HEAP_OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    tableSet(&vm.interned_strings, string, CREATE_NIL_VAL);

    return string;
}

static uint32_t hashString(const char* key, unsigned length) {
    //FNV-1a hash function
    uint32_t hash = 2166136261u;

    for (unsigned i = 0; i < length; i++) {
        hash ^= (unsigned)key[i];
        hash *= 16777619;
    }

    return hash;
}

HeapObjString* takeString(char* chars, unsigned length) {
    uint32_t hash = hashString(chars, length);
    HeapObjString* interned = tableFindString(&vm.interned_strings, chars, length, hash);
    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}

HeapObjString* copyString(const char* chars, unsigned length) {
    uint32_t hash = hashString(chars, length);
    HeapObjString* interned = tableFindString(&vm.interned_strings, chars, length, hash);
    if (interned != NULL) return interned;

    char* heapChars = ALLOCATE(char, (unsigned)length + 1);
    memcpy(heapChars, chars, (unsigned)length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

void printObject(Value value) {
    switch (GET_TYPE_OF_HEAP_OBJ(value)) {
        case HEAP_OBJ_STRING:
            printf("%s", READ_VALUE_AS_CSTRING(value));
            break;
    }
}
