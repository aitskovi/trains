#include <event.h>

#include <log.h>
#include <task.h>

#define INVALID_EVENT -1
#define CORRUPTED_VOLATILE_DATA -2

#define NO_WAITERS -1
#define TOO_MANY_WAITERS -4

static Task *waiters[NUM_EVENTS];

int kawait(Task *task, int event) {
   // Verify the event is valid.
   if (event < 0 || event >= NUM_EVENTS) {
       task_set_return_value(task, INVALID_EVENT);
       return INVALID_EVENT;
   }

   if (waiters[event]) return TOO_MANY_WAITERS;

   waiters[event] = task;
   task->state = EVT_BLOCKED;

   return 0;
}

int kevent(int event, int data) {
    if (!waiters[event]) {
        dlog("Recieved the event:%d, with no waiters\n", event);
        return NO_WAITERS;
    }

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
