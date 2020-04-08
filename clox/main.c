#include "common.h"
#include "chunk.h"
#include "debug.h"

int main(int argc, const char* argv[]){
    Chunk chunk;
    initChunk(&chunk);

    ValueIndex constant_index = addConstant(&chunk, 1.2);
    pushBackChunk(&chunk, OP_CONSTANT, 123);
    pushBackChunk(&chunk, (ChunkItem)constant_index, 123);

    pushBackChunk(&chunk, OP_RETURN, 123);

    disassembleChunk(&chunk, "test chunk");
    freeChunk(&chunk);

    return 0;
}
