TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

HEADERS += chunk.h \
           common.h \
           compiler.h \
           debug.h \
           memory.h \
           object.h \
           scanner.h \
           table.h \
           value.h \
           vm.h

SOURCES += main.c \
           chunk.c \
           compiler.c \
           debug.c \
           memory.c \
           object.c \
           scanner.c \
           table.c \
           value.c \
           vm.c
