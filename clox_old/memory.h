#ifndef clox_memory_h
#define clox_memory_h

#include "object.h"

#include <stddef.h> //defines size_t

#define ALLOCATE(type, count) \
	(type*)reallocate(NULL, 0, sizeof(type)*(count))

#define FREE(type, pointer) \
	reallocate(pointer, sizeof(type), 0)

#define GROW_CAPACITY(capacity) \
	( (capacity) < 8 ? 8 : (capacity)*2 ) //works on an integer
	
#define GROW_ARRAY(previous, type, oldCount, count) \
	(type*)reallocate(previous, sizeof(type) * (oldCount), \
		sizeof(type)*(count)) //actually grows the array
		
#define FREE_ARRAY(type, pointer, oldCount) \
	reallocate(pointer, sizeof(type) * (oldCount), 0)
		
void* reallocate(void* previous, size_t oldSize, size_t newSize);
void freeObjects();

#endif