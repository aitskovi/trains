#ifndef _EVENT_H_
#define _EVENT_H_

#include <ts7200.h>

struct Task;

enum event {
    TIMER_3_EVENT,
    NUM_EVENTS
};

/**
 * Wait for a specific event to occur.
 */
int kawait(struct Task *task, int event);

/**
 * Notify the event system an event has occured.
 */
int kevent(int event, int data);

#endif
