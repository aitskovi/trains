#include <interrupt.h>

#include <dassert.h>
#include <bits.h>
#include <log.h>
#include <uart.h>

#define NUM_VICS 2
#define VIC_SIZE 32
#define VIC_2_INT_ENABLE_BITS 0xf97865fb
#define VIC_1_INT_ENABLE_BITS 0x7f7ffff

void disable_interrupts() {
    *(int *)(VIC1_BASE + VIC_INT_EN_CLEAR_OFFSET) = VIC_1_INT_ENABLE_BITS;
    *(int *)(VIC2_BASE + VIC_INT_EN_CLEAR_OFFSET) = VIC_2_INT_ENABLE_BITS;
}

void initialize_interrupts() {
    // Disable all the interrupts to start.
    disable_interrupts();
}

int process_uart_interrupt(int channel, int *data) {
    int base = (channel == COM1) ? UART1_BASE : UART2_BASE;
    int interrupt = *(int *)(base + UART_INTR_OFFSET);

    int event;

    // Transmit Interrupt
    if (interrupt & TIS_MASK) {
        dlog("Transmit Interrupt\n");
        // We don't necessarily have something to write, so turn off
        // the transmit interrupt for now. It will get turned on again
        // when someone uses uart_write to write.
        uart_disable_interrupt(channel, T_INTERRUPT);
        return channel == COM1 ? UART_1_TX_EVENT : UART_2_TX_EVENT;
    }

    // Modem Interrupt
    if (interrupt & MIS_MASK) {
        dlog("Modem Interrupt\n");
        dassert(channel != COM2, "Modem for Invalid Channel");

        // Clear the MIS Interrupt
        *(int *)(base + UART_INTR_OFFSET) &= MIS_MASK;

        // Return the CTS Event
        return UART_1_CTS_EVENT;
    }

    // Recieve Interrupt
    if (interrupt & RIS_MASK) {
        dlog("Recieve Interrupt\n");
    }

    // Recieve Timeout Interrupt
    if (interrupt & RTIS_MASK) {
        dlog("Timeout Interrupt\n");
    }

    return event;
}

int process_interrupt(int *data) {
    // Process one interrupt.
    int interrupt = 0;

    // Get the interrupt number of the fired interrupt.
    interrupt = ffs(*(int *)(VIC1_BASE + VIC_IRQ_STATUS_OFFSET));
    if (!interrupt) {
        interrupt = ffs(*(int *)(VIC2_BASE + VIC_IRQ_STATUS_OFFSET));
        if (!interrupt) return -1;
        interrupt += 32;
    }
    interrupt -= 1;

    switch(interrupt) {
        case TIMER_3_INTERRUPT:
            dlog("Recieved Timer Interrupt\n");
            clear_interrupt(interrupt);
            return TIMER_3_EVENT;
        case UART_2_INTERRUPT:
            dlog("Recieved UART2 Interrupt\n");
            return process_uart_interrupt(COM2, data);
        case UART_1_INTERRUPT:
            dlog("Recieved UART1 Interrupt\n");
            return process_uart_interrupt(COM1, data);
        default:
            log("Received Unrecognized Interrupt: %d\n", interrupt);
    }

    return -1;
}

int disable_interrupt(enum interrupt interrupt) {
    int base = interrupt < 32 ? VIC1_BASE : VIC2_BASE;

    int shift = interrupt < 32 ? interrupt : interrupt - 32;
    int bit = 1 << shift;

    int *disabled = (int *)(base + VIC_INT_EN_CLEAR_OFFSET);
    *disabled |= bit;

    return 0;
}

int enable_interrupt(enum interrupt interrupt) {
    int base = interrupt < 32 ? VIC1_BASE : VIC2_BASE;

    int shift = interrupt < 32 ? interrupt : interrupt - 32;
    int bit = 1 << shift;

    int *enabled = (int *)(base + VIC_INT_ENABLE_OFFSET);
    *enabled |= bit;
    
    return 0;
}

int clear_software_interrupt(enum interrupt interrupt) {
    int base = interrupt < 32 ? VIC1_BASE : VIC2_BASE;

    int shift = interrupt < 32 ? interrupt : interrupt - 32;
    int bit = 1 << shift;

    int *clear = (int *)(base + VIC_SOFT_INT_CLEAR_OFFSET);
    *clear |= bit;

    return 0;
}

int clear_interrupt(enum interrupt interrupt) {
    switch(interrupt) {
        case TIMER_3_INTERRUPT: {
            int *clear = (int *)TIMER_3_CLEAR;
            *clear = 1;
            clear_software_interrupt(interrupt);
            return TIMER_3_EVENT;
        }
        default:
            log("Received Unrecognized Interrupt: %d\n", interrupt);
    }

    return -1;

}

int generate_interrupt(enum interrupt interrupt) {
    int base = interrupt < 32 ? VIC1_BASE : VIC2_BASE;

    int shift = interrupt < 32 ? interrupt : interrupt - 32;
    int bit = 1 << shift;

    int *generate = (int *)(base + VIC_SOFT_INT_OFFSET);
    *generate |= bit;

    return 0;
}
