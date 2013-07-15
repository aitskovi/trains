#include <log.h>
#include <bwio.h>
#include <write_server.h>
#include <sprintf.h>
#include <string.h>
#include <shell.h>

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
    char command[250];
    char *pos = &command[0];
    pos += sprintf(pos, "\0337\033[%u;%uH\n", CONSOLE_HEIGHT + SCROLLABLE_AREA_SIZE, 1);
    pos += sformat(pos, fmt, va);
    pos += sprintf(pos, "\0338");
    Write(COM2, command, pos - command);
    va_end(va);
}
