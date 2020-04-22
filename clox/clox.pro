TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        chunk.c \
        compiler.c \
        debug.c \
        hashtable.c \
        main.c \
        memory.c \
        object.c \
        scanner.c \
        value.c \
        vm.c

HEADERS += \
    chunk.h \
    common.h \
    compiler.h \
    debug.h \
    hashtable.h \
    memory.h \
    object.h \
    scanner.h \
    value.h \
    vm.h
