#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(ChunkItem, chunk->code, chunk->capacity);
    FREE_ARRAY(unsigned, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

void pushBackChunk(Chunk* chunk, ChunkItem byte, LineNumber line) {
    if (chunk->capacity < chunk->count + 1) {
        ChunkIndex oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(chunk->code, ChunkItem, oldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(chunk->lines, unsigned, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

ValueIndex addConstant(Chunk* chunk, Value value) {
    pushBackValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}
