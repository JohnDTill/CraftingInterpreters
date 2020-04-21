#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"

#define GET_TYPE_OF_HEAP_OBJ(value)      (READ_VALUE_UNION_AS_HEAP_OBJ(value)->type)

#define HEAP_OBJ_IS_STRING(value)        isObjType(value, HEAP_OBJ_STRING)

#define READ_VALUE_AS_STRING(value)        ((HeapObjString*)READ_VALUE_UNION_AS_HEAP_OBJ(value))
#define READ_VALUE_AS_CSTRING(value)       (READ_VALUE_AS_STRING(value)->chars)

typedef enum {
    HEAP_OBJ_STRING,
} HeapObjType;

struct sHeapObj {
    HeapObjType type;
    struct sHeapObj* next;
};

struct sHeapObjString {
    HeapObj obj;
    int length;
    char* chars;
};

HeapObjString* takeString(char* chars, int length);
HeapObjString* copyString(const char* chars, int length);

void printObject(Value value);

inline bool isObjType(Value value, HeapObjType type) {
    return VALUE_TYPE_IS_HEAP_OBJ(value) && GET_TYPE_OF_HEAP_OBJ(value) == type;
}

#endif
