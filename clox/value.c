#include <stdio.h>
#include <string.h>

#include "object.h"
#include "memory.h"
#include "value.h"

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;

    switch (a.type) {
        case VAL_BOOL:   return READ_VALUE_UNION_AS_BOOL(a) == READ_VALUE_UNION_AS_BOOL(b);
        case VAL_NIL:    return true;
        case VAL_NUMBER: return READ_VALUE_UNION_AS_NUMBER(a) == READ_VALUE_UNION_AS_NUMBER(b);
        //Floating-point equality is dubious, and Lox only supports double numbers, but it is a toy language!
        case VAL_HEAP_OBJ: {
            HeapObjString* aString = READ_VALUE_AS_STRING(a);
            HeapObjString* bString = READ_VALUE_AS_STRING(b);
            return aString->length == bString->length &&
            memcmp(aString->chars, bString->chars, (unsigned)aString->length) == 0;
        }
    }
}

void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void pushBackValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        ValueIndex oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(array->values, Value, oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void printValue(Value value) {
    switch (value.type) {
        case VAL_BOOL:   printf(READ_VALUE_UNION_AS_BOOL(value) ? "true" : "false"); break;
        case VAL_NIL:    printf("nil"); break;
        case VAL_NUMBER: printf("%g", READ_VALUE_UNION_AS_NUMBER(value)); break;
        case VAL_HEAP_OBJ:    printObject(value); break;
    }
}
