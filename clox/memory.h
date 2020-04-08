#ifndef clox_memory_h
#define clox_memory_h

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(previous, type, oldCount, count) \
    (type*)reallocate(previous, sizeof(type) * (oldCount), \
        sizeof(type) * (count))

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

//Routing all allocations through a single function is important
//for the garbage collector that keeps track of how much memory is in use.
void* reallocate(void* previous, size_t oldSize, size_t newSize);

#endif
