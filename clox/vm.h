#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "hashtable.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;
    ChunkItem* ip; //Points to next instruction to be executed
    Value stack[STACK_MAX];
    Value* stackTop; //Address past last element
    HashTable interned_strings;

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
bool isFalsey(Value value);

#endif
