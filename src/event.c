#include <event.h>

#include <interrupt.h>
#include <log.h>
#include <task.h>

#define INVALID_EVENT -1
#define CORRUPTED_VOLATILE_DATA -2

#define NO_WAITERS -1
#define TOO_MANY_WAITERS -4

static Task *waiters[NUM_EVENTS];

int kawait(Task *task, int event) {
   // Verify the event is valid.
   if (!is_valid_event(event)) {
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

int is_valid_event(int event) {
   return (event >= 0 || event < NUM_EVENTS);
}

int enable_event(int event) {
    if (!is_valid_event(event)) return INVALID_EVENT;

    switch(event) {
        case TIMER_3_EVENT:
            enable_interrupt(TIMER_3_INTERRUPT);
            break;
        default:
            log("Enabling Invalid Interrupt\n");
            break;
    }

    return 0;
}

void initialize_events() {
    initialize_interrupts();

    int i;
    for (i = 0; i < NUM_EVENTS; ++i) {
        waiters[i] = 0;
    }
}
