#ifndef _EVENT_H_
#define _EVENT_H_

#include <ts7200.h>

struct Task;

enum event {
    TIMER_3_EVENT,
    UART_1_RCV_EVENT,
    UART_1_TX_EVENT,
    UART_1_CTS_EVENT,
    UART_2_RCV_EVENT,
    UART_2_TX_EVENT,
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

/**
 * Verify if the event is valid.
 */
int is_valid_event(int event);

/**
 * Enable an event. This ensures that the correct interrupts are
 * turned on, so the event will fire.
 */
int enable_event(int event);

/**
 * Initialize the event system.
 */
void initialize_events();

#endif
