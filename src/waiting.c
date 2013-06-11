#include <waiting.h>

#include <dassert.h>
#include <task.h>
#include <scheduling.h>

static Task *waiters[MAX_TASKS];

int waiting_add(Task *task, Task *waiter) {
    if (task->state == ZOMBIE) return NO_WAIT;

    // Insert the waiter.
    waiter->next = waiters[task->tid];
    waiters[task->tid] = waiter;

    return 0;
}

Task *waiting_pop(Task *task) {
    dassert(task->state == ZOMBIE, "Popping waiters from non zombie task");

    if (waiters[task->tid] == 0) return 0;

    Task *t = waiters[task->tid];
    waiters[task->tid] = t->next;
    t->next = 0;

    return t;
}

void initialize_waiting() {
    int i;
    for (i = 0; i < MAX_TASKS; ++i) {
        waiters[i] = 0;
    }
}
