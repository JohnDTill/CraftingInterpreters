#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "hashtable.h"
#include "object.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
    HeapObjClosure* closure;
    uint8_t* ip;
    Value* slots;
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    Value stack[STACK_MAX];
    Value* stackTop; //Address past last element
    HashTable globals;
    HashTable interned_strings;
    ObjUpvalue* openUpvalues;

    HeapObj* objects; //Pointer to head of linked list for memory management
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM(void);
void freeVM(void);
InterpretResult interpret(const char* source);
void pushStack(Value value);
Value popStack(void);
Value peekStack(int distance);
bool callValue(Value callee, int argCount);
ObjUpvalue* captureUpvalue(Value* local);
void closeUpvalues(Value* last);
bool isFalsey(Value value);

#endif
