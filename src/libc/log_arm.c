#include <log.h>
#include <bwio.h>
#include <write_server.h>

#define MAX_PRINTABLE 200

void log(char *fmt, ...) {
    va_list va;

    va_start(va,fmt);
    bwformat(COM2 , fmt, va );
    va_end(va);
}

void ulog(char *fmt, ...) {
    va_list va;

    va_start(va,fmt);
    char buf[200];
    int size = sformat(buf, fmt, va);

    if (size < 0) {
        Write(COM2, "FATAL: Ulog Failed\n", strlen("FATAL: Ulog Failed"));
    }

    Write(COM2, buf, size);
    va_end(va);

}
