#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef double Value;
typedef unsigned ValueIndex;

typedef struct {
    ValueIndex capacity; //changed from int
    ValueIndex count; //changed from int
    Value* values;
} ValueArray;

void initValueArray(ValueArray* array);
void pushBackValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);

#endif
