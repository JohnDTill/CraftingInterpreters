#include <stdio.h>

#include "debug.h"
#include "value.h"

void disassembleChunk(Chunk* chunk, const char* name) {
    printf("== %s ==\n", name);

    for (ChunkIndex offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

static ChunkIndex constantInstruction(const char* name, Chunk* chunk, ChunkIndex offset) {
    ChunkItem constant = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");

    return offset + 2;
}

static unsigned simpleInstruction(const char* name, ChunkIndex offset) {
    printf("%s\n", name);
    return offset + 1;
}

ChunkIndex disassembleInstruction(Chunk* chunk, ChunkIndex offset) {
    printf("%04d ", offset);
    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", chunk->lines[offset]);
    }

    ChunkItem instruction = chunk->code[offset];
    switch (instruction) {
        case OP_CONSTANT:
              return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
