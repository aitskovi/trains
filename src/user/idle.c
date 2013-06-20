#include <idle.h>

#include <log.h>
#include <syscall.h>

void idle() {
    dlog("Idle task started with tid %u\n", MyTid());

    for(;;) {
    }

    Exit();
}
