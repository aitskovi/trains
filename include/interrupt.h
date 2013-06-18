#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include <event.h>
#include <ts7200.h>

enum interrupt {
    TIMER_3_INTERRUPT = 51,
    UART_1_INTERRUPT = 52,
    UART_2_INTERRUPT = 54,
};

enum clear {
    TIMER_3_CLEAR = TIMER3_BASE + CLR_OFFSET,
};

/**
 * Initialize the interrupt system.
 *
 * Disables all interrupts, we later enable the ones we want
 * selectively.
 */
void initialize_interrupts();

void disable_interrupts();

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
