#ifndef _UART_H_
#define _UART_H_

enum uart_interrupt {
    T_INTERRUPT,
    R_INTERRUPT,
    MS_INTERRUPT,
    RT_INTERRUPT,
    UART_NUM_INTERRUPTS,
};

/**
 * Setup a uart properly.
 */
void uart_initialize(int channel);

/**
 * Write some data to a UART.
 */
int uart_write(int port, char c);

/**
 * Read some data from a UART.
 */
int uart_read(int port);

/**
 * Disable an interrupt.
 */
int uart_disable_interrupt(int channel, enum uart_interrupt);

/**
 * Enable a uart interrupt.
 */
int uart_enable_interrupt(int channel, enum uart_interrupt interrupt);

int uart_setfifo(int channel, int state);
int uart_setspeed(int channel, int speed);
int uart_setstop(int channel, int num);

#endif
