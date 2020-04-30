#include <stdlib.h>

#include "common.h"
#include "memory.h"
#include "vm.h"

void* reallocate(void* previous, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        free(previous);
        return NULL;
    }

    return realloc(previous, newSize);
}

static void freeObject(HeapObj* object) {
    switch (object->type) {
        case HEAP_OBJ_CLOSURE: {
            HeapObjClosure* closure = (HeapObjClosure*)object;
            FREE_ARRAY(ObjUpvalue*, closure->upvalues, closure->upvalueCount);
            FREE(HeapObjClosure, object);
            break;
        }
        case HEAP_OBJ_FUNCTION: {
            HeapObjFunction* function = (HeapObjFunction*)object;
            freeChunk(&function->chunk);
            FREE(HeapObjFunction, object);
            break;
        }
        case HEAP_OBJ_NATIVE:
            FREE(HeapObjNative, object);
            break;
        case HEAP_OBJ_STRING: {
            HeapObjString* string = (HeapObjString*)object;
            FREE_ARRAY(char, string->chars, (unsigned)string->length + 1);
            FREE(HeapObjString, object);
            break;
        }
        case HEAP_OBJ_UPVALUE:
            FREE(ObjUpvalue, object);
            break;
    }
}

void freeObjects() {
    HeapObj* object = vm.objects;
    while (object != NULL) {
        HeapObj* next = object->next;
        freeObject(object);
        object = next;
    }
}
