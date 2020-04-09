#include <stdio.h>

#include "debug.h"
#include "value.h"

void disassembleChunk(Chunk* chunk, const char* name) {
    printf("== %s ==\n", name);
    printf("Index, Line, OpCode,          Info\n");

    for (ChunkIndex offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

static ChunkIndex constantInstruction(const char* name, Chunk* chunk, ChunkIndex offset) {
    ChunkItem constant_index = chunk->code[offset + 1];
    printf("%-16s ValueIndex: %d, Value = ", name, constant_index);
    printValue(chunk->constants.values[constant_index]);
    printf("\n");

    return offset + 2;
}

static unsigned simpleInstruction(const char* name, ChunkIndex offset) {
    printf("%s\n", name);
    return offset + 1;
}

ChunkIndex disassembleInstruction(Chunk* chunk, ChunkIndex offset) {
    printf(" %04d  ", offset);
    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
        printf("   |  ");
    } else {
        printf("%4d  ", chunk->lines[offset]);
    }

    ChunkItem instruction = chunk->code[offset];
    switch (instruction) {
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);
        case OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);
        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
