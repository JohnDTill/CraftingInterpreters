TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

#DEFINES += DEBUG_PRINT_CODE \
#           DEBUG_TRACE_EXECUTION

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

DISTFILES += test.txt

# copies the given files to the destination directory
defineTest(copyToDestDir) {
    files = $$1
    dir = $$2
    # replace slashes in destination path for Windows
    win32:dir ~= s,/,\\,g

    for(file, files) {
        # replace slashes in source path for Windows
        win32:file ~= s,/,\\,g

        QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$shell_quote($$file) $$shell_quote($$dir) $$escape_expand(\\n\\t)
    }

    export(QMAKE_POST_LINK)
}

copyToDestDir($$PWD/test.txt, $$OUT_PWD)
