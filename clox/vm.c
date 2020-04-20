#include <stdarg.h>
#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

static VM vm;

static void resetStack() {
    vm.stackTop = vm.stack;
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args); //static analyzer wants format=>"%s", doesn't like variable format
    va_end(args);
    fputs("\n", stderr);

    ChunkIndex instruction = (unsigned)(vm.ip - vm.chunk->code) - 1;
    LineNumber line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);

    resetStack();
}

void initVM() {
    resetStack();
}

void freeVM() {
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

#define BINARY_OP(valueType, op) \
    do { \
      if (!VALUE_TYPE_IS_NUMBER(peekStack(0)) || !VALUE_TYPE_IS_NUMBER(peekStack(1))) { \
        runtimeError("Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      \
      double b = READ_VALUE_UNION_AS_NUMBER(popStack()); \
      double a = READ_VALUE_UNION_AS_NUMBER(popStack()); \
      pushStack(valueType(a op b)); \
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
            case OP_NIL: pushStack(CREATE_NIL_VAL); break;
            case OP_TRUE: pushStack(CREATE_BOOL_VAL(true)); break;
            case OP_FALSE: pushStack(CREATE_BOOL_VAL(false)); break;

            case OP_EQUAL: {
                Value b = popStack();
                Value a = popStack();
                pushStack(CREATE_BOOL_VAL(valuesEqual(a, b)));
                break;
            }

            case OP_GREATER:  BINARY_OP(CREATE_BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(CREATE_BOOL_VAL, <); break;
            case OP_ADD:      BINARY_OP(CREATE_NUMBER_VAL, +); break;
            case OP_SUBTRACT: BINARY_OP(CREATE_NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(CREATE_NUMBER_VAL, *); break;
            case OP_DIVIDE:   BINARY_OP(CREATE_NUMBER_VAL, /); break;
            case OP_NOT:
                    pushStack(CREATE_BOOL_VAL(isFalsey(popStack())));
                    break;
            case OP_NEGATE:
                    if (!VALUE_TYPE_IS_NUMBER(peekStack(0))) {
                        runtimeError("Operand must be a number.");
                        return INTERPRET_RUNTIME_ERROR;
                    }

                    //This has more indirection than necessary since stack entries are immutable
                    pushStack(CREATE_NUMBER_VAL(-READ_VALUE_UNION_AS_NUMBER(popStack())));
                    break;
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

InterpretResult interpret(const char* source) {
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = run();

    freeChunk(&chunk);
    return result;
}

void pushStack(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value popStack() {
    vm.stackTop--;
    return *vm.stackTop;
}

Value peekStack(int distance) {
  return vm.stackTop[-1 - distance];
}

bool isFalsey(Value value) {
  return VALUE_TYPE_IS_NIL(value) || (VALUE_TYPE_IS_BOOL(value) && !READ_VALUE_UNION_AS_BOOL(value));
}
