#include <log.h>
#include <bwio.h>

void log(char *fmt, ...) {
    va_list va;

    va_start(va,fmt);
    bwformat(COM2 , fmt, va );
    va_end(va);
}
