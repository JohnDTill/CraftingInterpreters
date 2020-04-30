#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"

VM vm;

static Value clockNative(int argCount, Value* args) {
    return CREATE_NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static void resetStack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
    vm.openUpvalues = NULL;
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args); //static analyzer wants format=>"%s", doesn't like variable format
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm.frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &vm.frames[i];
        HeapObjFunction* function = frame->closure->function;
        // -1 because the IP is sitting on the next instruction to be
        // executed.
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ",
        function->chunk.lines[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    resetStack();
}

static void defineNative(const char* name, NativeFn function) {
    pushStack(CREATE_HEAP_OBJ_VAL(copyString(name, (unsigned)strlen(name))));
    pushStack(CREATE_HEAP_OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, READ_VALUE_AS_STRING(vm.stack[0]), vm.stack[1]);
    popStack();
    popStack();
}

void initVM() {
    resetStack();
    vm.objects = NULL;
    initTable(&vm.globals);
    initTable(&vm.interned_strings);

    defineNative("clock", clockNative);
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
    CallFrame* frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_BYTE()])
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
        disassembleInstruction(&frame->closure->function->chunk,
                               (ChunkIndex)(frame->ip - frame->closure->function->chunk.code));
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

            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                pushStack(*frame->closure->upvalues[slot]->location);
                break;
            }

            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = peekStack(0);
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
                pushStack(frame->slots[slot]);
                break;
            }

            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peekStack(0);
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
                frame->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peekStack(0))) frame->ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }
            case OP_CALL: {
                int argCount = READ_BYTE();
                if (!callValue(peekStack(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_CLOSURE: {
                HeapObjFunction* function = READ_VALUE_AS_FUNCTION(READ_CONSTANT());
                HeapObjClosure* closure = newClosure(function);
                pushStack(CREATE_HEAP_OBJ_VAL(closure));
                for (int i = 0; i < closure->upvalueCount; i++) {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    if (isLocal) {
                        closure->upvalues[i] = captureUpvalue(frame->slots + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;
            }
            case OP_CLOSE_UPVALUE:
                 closeUpvalues(vm.stackTop - 1);
                 popStack();
                 break;
            case OP_RETURN: {
                Value result = popStack();

                closeUpvalues(frame->slots);

                vm.frameCount--;
                if (vm.frameCount == 0) {
                    popStack();
                    return INTERPRET_OK;
                }

                vm.stackTop = frame->slots;
                pushStack(result);

                frame = &vm.frames[vm.frameCount - 1];
                break;
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
    HeapObjFunction* function = compile(source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    pushStack(CREATE_HEAP_OBJ_VAL(function));
    HeapObjClosure* closure = newClosure(function);
    popStack();
    pushStack(CREATE_HEAP_OBJ_VAL(closure));
    callValue(CREATE_HEAP_OBJ_VAL(closure), 0);

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

Value peekStack(int distance) {
    return vm.stackTop[-1 - distance];
}

static bool call(HeapObjClosure* closure, int argCount) {
    if (argCount != closure->function->arity) {
        runtimeError("Expected %d arguments but got %d.", closure->function->arity, argCount);
        return false;
    }

    if (vm.frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }

    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;

    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

bool callValue(Value callee, int argCount) {
    if (VALUE_TYPE_IS_HEAP_OBJ(callee)) {
        switch (GET_TYPE_OF_HEAP_OBJ(callee)) {
            case HEAP_OBJ_CLOSURE:
                return call(READ_VALUE_AS_CLOSURE(callee), argCount);
            case HEAP_OBJ_NATIVE: {
                NativeFn native = READ_VALUE_AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop - argCount);
                vm.stackTop -= argCount + 1;
                pushStack(result);
                return true;
            }
            default:
                // Non-callable object type.
            break;
        }
    }

    runtimeError("Can only call functions and classes.");
    return false;
}

ObjUpvalue* captureUpvalue(Value* local) {
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm.openUpvalues;

    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) return upvalue;

    ObjUpvalue* createdUpvalue = newUpvalue(local);
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

void closeUpvalues(Value* last) {
    while (vm.openUpvalues != NULL && vm.openUpvalues->location >= last) {
        ObjUpvalue* upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}

bool isFalsey(Value value) {
    return VALUE_TYPE_IS_NIL(value) || (VALUE_TYPE_IS_BOOL(value) && !READ_VALUE_UNION_AS_BOOL(value));
}
