#include <idle.h>

#include <log.h>
#include <syscall.h>

void idle() {
    log("Idle: Initializing\n");

    for(;;) {
    }

    Exit();
}
