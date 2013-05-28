#include <user.h>

#include <bwio.h>
#include <syscall.h>
#include <task.h>
#include <time.h>

void first() {
    int tid;
    tid = Create(LOW, second);
    bwprintf(COM2, "Created: <%d>\n", tid);
    tid = Create(LOW, second);
    bwprintf(COM2, "Created: <%d>\n", tid);
    tid = Create(HIGH, second);
    bwprintf(COM2, "Created: <%d>\n", tid);
    tid = Create(HIGH, second);
    bwprintf(COM2, "Created: <%d>\n", tid);
    bwprintf(COM2, "First: exiting\n");
    Exit();
}

void second() {
    int tid = MyTid();
    int parent_tid = MyParentTid();
    bwprintf(COM2, "My Tid: %d, My Parent Tid: %d\n", tid, parent_tid);
    Pass();
    bwprintf(COM2, "My Tid: %d, My Parent Tid: %d\n", tid, parent_tid);
    Exit();
}

void kernel_timing() {
    Timer timer;

    unsigned int i;
    for (i = 0; i < 4; ++i) {
        timer_reset(&timer);
        Pass();
        Time elapsed = timer_elapsed(&timer);
        bwprintf(COM2, "Round trip took %u usec\n", elapsed.useconds);
    }
    Exit();
}
