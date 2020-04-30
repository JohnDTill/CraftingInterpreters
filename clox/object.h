#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "chunk.h"
#include "value.h"

#define GET_TYPE_OF_HEAP_OBJ(value)      (READ_VALUE_UNION_AS_HEAP_OBJ(value)->type)

#define HEAP_OBJ_IS_CLOSURE(value)       isObjType(value, HEAP_OBJ_CLOSURE)
#define HEAP_OBJ_IS_FUNCTION(value)      isObjType(value, HEAP_OBJ_FUNCTION)
#define HEAP_OBJ_IS_NATIVE(value)        isObjType(value, HEAP_OBJ_NATIVE)
#define HEAP_OBJ_IS_STRING(value)        isObjType(value, HEAP_OBJ_STRING)

#define READ_VALUE_AS_CLOSURE(value)       ((HeapObjClosure*)READ_VALUE_UNION_AS_HEAP_OBJ(value))
#define READ_VALUE_AS_FUNCTION(value)      ((HeapObjFunction*)READ_VALUE_UNION_AS_HEAP_OBJ(value))
#define READ_VALUE_AS_NATIVE(value)        (((HeapObjNative*)READ_VALUE_UNION_AS_HEAP_OBJ(value))->function)
#define READ_VALUE_AS_STRING(value)        ((HeapObjString*)READ_VALUE_UNION_AS_HEAP_OBJ(value))
#define READ_VALUE_AS_CSTRING(value)       (READ_VALUE_AS_STRING(value)->chars)

typedef enum {
    HEAP_OBJ_CLOSURE,
    HEAP_OBJ_FUNCTION,
    HEAP_OBJ_NATIVE,
    HEAP_OBJ_STRING,
    HEAP_OBJ_UPVALUE
} HeapObjType;

struct sHeapObj {
    HeapObjType type;
    struct sHeapObj* next;
};

typedef struct {
  HeapObj obj;
  int arity;
  unsigned upvalueCount;
  Chunk chunk;
  HeapObjString* name;
} HeapObjFunction;

typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
    HeapObj obj;
    NativeFn function;
} HeapObjNative;

struct sHeapObjString {
    HeapObj obj;
    unsigned length;
    char* chars;
    uint32_t hash;
};

typedef struct sUpvalue {
    HeapObj obj;
    Value* location;
    Value closed;
    struct sUpvalue* next;
} ObjUpvalue;

typedef struct {
  HeapObj obj;
  HeapObjFunction* function;
  ObjUpvalue** upvalues;
  unsigned upvalueCount;
} HeapObjClosure;

HeapObjClosure* newClosure(HeapObjFunction* function);
HeapObjFunction* newFunction(void);
HeapObjNative* newNative(NativeFn function);
HeapObjString* takeString(char* chars, unsigned length);
HeapObjString* copyString(const char* chars, unsigned length);
ObjUpvalue* newUpvalue(Value* slot);
void printObject(Value value);

bool isObjType(Value value, HeapObjType type);

#endif
