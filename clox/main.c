#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]){
    initVM();

    Chunk chunk;
    initChunk(&chunk);

    ValueIndex constant_index = addConstant(&chunk, 1.2);
    pushBackChunk(&chunk, OP_CONSTANT, 123);
    pushBackChunk(&chunk, (ChunkItem)constant_index, 123);

    constant_index = addConstant(&chunk, 3.4);
    pushBackChunk(&chunk, OP_CONSTANT, 123);
    pushBackChunk(&chunk, (ChunkItem)constant_index, 123);

    pushBackChunk(&chunk, OP_ADD, 123);

    constant_index = addConstant(&chunk, 5.6);
    pushBackChunk(&chunk, OP_CONSTANT, 123);
    pushBackChunk(&chunk, (ChunkItem)constant_index, 123);

    pushBackChunk(&chunk, OP_DIVIDE, 123);

    pushBackChunk(&chunk, OP_NEGATE, 123);

    pushBackChunk(&chunk, OP_RETURN, 123);

    disassembleChunk(&chunk, "test chunk");
    interpret(&chunk);
    freeVM();
    freeChunk(&chunk);

    return 0;
}
