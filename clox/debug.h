#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"

void disassembleChunk(Chunk* chunk, const char* name);
ChunkIndex disassembleInstruction(Chunk* chunk, ChunkIndex offset);

#endif
