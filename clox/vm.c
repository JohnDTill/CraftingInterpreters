#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"

VM vm;

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
    vm.objects = NULL;
    initTable(&vm.globals);
    initTable(&vm.interned_strings);
}

void freeVM() {
    freeTable(&vm.globals);
    freeTable(&vm.interned_strings);
    freeObjects();
}

static void concatenate() {
    HeapObjString* b = READ_VALUE_AS_STRING(popStack());
    HeapObjString* a = READ_VALUE_AS_STRING(popStack());

    unsigned length = a->length + b->length;
    char* chars = ALLOCATE(char, (unsigned)length + 1);
    memcpy(chars, a->chars, (unsigned)a->length);
    memcpy(chars + a->length, b->chars, (unsigned)b->length);
    chars[length] = '\0';

    HeapObjString* result = takeString(chars, length);
    pushStack(CREATE_HEAP_OBJ_VAL(result));
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define READ_SHORT() (vm.ip += 2, (uint16_t)((vm.ip[-2] << 8) | vm.ip[-1]))
#define READ_STRING() READ_VALUE_AS_STRING(READ_CONSTANT())

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

            case OP_SET_GLOBAL: {
                HeapObjString* name = READ_STRING();
                if (tableSet(&vm.globals, name, peekStack(0))) {
                    tableDelete(&vm.globals, name);
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_EQUAL: {
                Value b = popStack();
                Value a = popStack();
                pushStack(CREATE_BOOL_VAL(valuesEqual(a, b)));
                break;
            }

            case OP_POP: popStack(); break;

            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                pushStack(vm.stack[slot]);
                break;
            }

            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                vm.stack[slot] = peekStack(0);
                break;
            }

            case OP_GET_GLOBAL: {
                HeapObjString* name = READ_STRING();
                Value value;
                if (!tableGet(&vm.globals, name, &value)) {
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                pushStack(value);
                break;
            }

            case OP_DEFINE_GLOBAL: {
                HeapObjString* name = READ_STRING();
                tableSet(&vm.globals, name, peekStack(0));
                popStack();
                break;
            }

            case OP_GREATER:  BINARY_OP(CREATE_BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(CREATE_BOOL_VAL, <); break;
            case OP_ADD: {
                if (HEAP_OBJ_IS_STRING(peekStack(0)) && HEAP_OBJ_IS_STRING(peekStack(1))) {
                    concatenate();
                } else if (VALUE_TYPE_IS_NUMBER(peekStack(0)) && VALUE_TYPE_IS_NUMBER(peekStack(1))) {
                    double b = READ_VALUE_UNION_AS_NUMBER(popStack());
                    double a = READ_VALUE_UNION_AS_NUMBER(popStack());
                    pushStack(CREATE_NUMBER_VAL(a + b));
                } else {
                    runtimeError("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
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
            case OP_PRINT: {
                printValue(popStack());
                printf("\n");
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                vm.ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peekStack(0))) vm.ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                vm.ip -= offset;
                break;
            }
            case OP_RETURN: {
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
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
