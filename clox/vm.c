#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "vm.h"

static VM vm;

static void resetStack() {
    vm.stackTop = vm.stack;
}

void initVM() {
    resetStack();
}

void freeVM() {
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

#define BINARY_OP(op) \
    do { \
      double b = popStack(); \
      double a = popStack(); \
      pushStack(a op b); \
    } while (false) //WHILE loop allows multi-statement macro to be used naturally with IF statements

    for (;;) {
        #ifdef DEBUG_TRACE_EXECUTION
        printf("          Stack State: [ ");
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            if(slot!=vm.stack) printf(" | ");
            printValue(*slot);
        }
        printf(" ]");
        printf("\n");
        disassembleInstruction(vm.chunk, (ChunkIndex)(vm.ip - vm.chunk->code));
        #endif

        OpCode instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                pushStack(constant);
                break;
            }
            case OP_ADD:      BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE:   BINARY_OP(/); break;
            case OP_NEGATE: pushStack(-popStack()); break; //This has more indirection than necessary
            case OP_RETURN: {
                printValue(popStack());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}

void pushStack(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value popStack() {
    vm.stackTop--;
    return *vm.stackTop;
}
