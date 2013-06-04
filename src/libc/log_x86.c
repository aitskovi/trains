#include <log.h>

#include <stdarg.h>
#include <stdio.h>

void log(char *fmt, ...) {
    va_list va;

    char buffer[256];
    va_start(va, fmt);
    vsprintf(buffer, fmt, va);
    printf(buffer);
    va_end(va);
}
