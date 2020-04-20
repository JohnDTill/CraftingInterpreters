#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
    } as;
} Value;

#define VALUE_TYPE_IS_BOOL(value)    ((value).type == VAL_BOOL)
#define VALUE_TYPE_IS_NIL(value)     ((value).type == VAL_NIL)
#define VALUE_TYPE_IS_NUMBER(value)  ((value).type == VAL_NUMBER)

#define READ_VALUE_UNION_AS_BOOL(value)    ((value).as.boolean)
#define READ_VALUE_UNION_AS_NUMBER(value)  ((value).as.number)

#define CREATE_BOOL_VAL(data)   ((Value){ VAL_BOOL, { .boolean = data } })
#define CREATE_NIL_VAL           ((Value){ VAL_NIL, { .number = 0 } })
#define CREATE_NUMBER_VAL(data) ((Value){ VAL_NUMBER, { .number = data } })

typedef unsigned ValueIndex;

typedef struct {
    ValueIndex capacity; //changed from int
    ValueIndex count; //changed from int
    Value* values;
} ValueArray;

bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray* array);
void pushBackValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);

#endif
