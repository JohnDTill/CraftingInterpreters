TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        chunk.c \
        compiler.c \
        debug.c \
        main.c \
        memory.c \
        scanner.c \
        value.c \
        vm.c

HEADERS += \
    chunk.h \
    common.h \
    compiler.h \
    debug.h \
    memory.h \
    scanner.h \
    value.h \
    vm.h
