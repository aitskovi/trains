#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include <event.h>

enum interrupt {
    TIMER_3_INTERRUPT = 51,
};

enum clear {
    TIMER_3_CLEAR = TIMER3_BASE + CLR_OFFSET,
};

/**
 * Based on the interrupt, find what event has occured
 * and return it along with the volatile data. This also
 * clears the interrupt.
 */
int process_interrupt(int *data);

/**
 * Enable a specific interrupt.
 */
int enable_interrupt(enum interrupt);

/**
 * Clear a specific interrupt.
 */
int clear_interrupt(enum interrupt);

/**
 * Generate a specific interrupt.
 */
int generate_interrupt(enum interrupt);

#endif
