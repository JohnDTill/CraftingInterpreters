#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef struct sHeapObj HeapObj;
typedef struct sHeapObjString HeapObjString;

typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_HEAP_OBJ
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        HeapObj* obj;
    } as;
} Value;

#define VALUE_TYPE_IS_BOOL(value)     ((value).type == VAL_BOOL)
#define VALUE_TYPE_IS_NIL(value)      ((value).type == VAL_NIL)
#define VALUE_TYPE_IS_NUMBER(value)   ((value).type == VAL_NUMBER)
#define VALUE_TYPE_IS_HEAP_OBJ(value) ((value).type == VAL_HEAP_OBJ)

#define READ_VALUE_UNION_AS_HEAP_OBJ(value)  ((value).as.obj)
#define READ_VALUE_UNION_AS_BOOL(value)      ((value).as.boolean)
#define READ_VALUE_UNION_AS_NUMBER(value)    ((value).as.number)

#define CREATE_BOOL_VAL(data)         ((Value){ VAL_BOOL, { .boolean = data } })
#define CREATE_NIL_VAL                ((Value){ VAL_NIL, { .number = 0 } })
#define CREATE_NUMBER_VAL(data)       ((Value){ VAL_NUMBER, { .number = data } })
#define CREATE_HEAP_OBJ_VAL(object)   ((Value){ VAL_HEAP_OBJ, { .obj = (HeapObj*)object } })

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
