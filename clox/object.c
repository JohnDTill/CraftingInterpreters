#include <stdio.h>
#include <string.h>

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

static HeapObjString* allocateString(char* chars, int length) {
    HeapObjString* string = ALLOCATE_OBJ(HeapObjString, HEAP_OBJ_STRING);
    string->length = length;
    string->chars = chars;

    return string;
}

HeapObjString* takeString(char* chars, int length) {
    return allocateString(chars, length);
}

HeapObjString* copyString(const char* chars, int length) {
    char* heapChars = ALLOCATE(char, (unsigned)length + 1);
    memcpy(heapChars, chars, (unsigned)length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length);
}

void printObject(Value value) {
  switch (GET_TYPE_OF_HEAP_OBJ(value)) {
    case HEAP_OBJ_STRING:
      printf("%s", READ_VALUE_AS_CSTRING(value));
      break;
  }
}
