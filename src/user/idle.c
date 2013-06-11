#include <idle.h>

#include <log.h>
#include <syscall.h>

void idle() {
    log("Idle task started with tid %u\n", MyTid());

    for(;;) {
    }

    Exit();
}
