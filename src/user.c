#include <user.h>

#include <bwio.h>
#include <syscall.h>
#include <task.h>

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
