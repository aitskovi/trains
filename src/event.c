#include <event.h>

#include <task.h>

#define NO_WAITERS -1
#define TOO_MANY_WAITERS -4

static Task *waiters[NUM_EVENTS];

int kawait(Task *task, int event) {
   // Verify the event is valid.

   if (waiters[event]) return TOO_MANY_WAITERS;

   waiters[event] = task;
   task->state = EVT_BLOCKED;

   return 0;
}

int kevent(int event, int data) {
    if (!waiters[event]) return NO_WAITERS;

    Task *task = waiters[event];
    task_set_return_value(task, data);
    make_ready(task);

    waiters[event] = 0;
    
    return 0;
}

void initialize_events() {
    int i;
    for (i = 0; i < NUM_EVENTS; ++i) {
        waiters[i] = 0;
    }
}
