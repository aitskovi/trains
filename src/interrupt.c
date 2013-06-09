#include <interrupt.h>

#include <bits.h>
#include <log.h>

#define NUM_VICS 2
#define VIC_SIZE 32

void initialize_interrupts() {

    // Disable all the interrupts to start.
    int i;
    for (i = 0; i < NUM_VICS + VIC_SIZE; ++i) {
        disable_interrupt(i);
    }
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
        default:
            log("Received Unrecognized Interrupt: %d\n", interrupt);
    }

    return -1;
}

int disable_interrupt(enum interrupt interrupt) {
    int base = interrupt < 32 ? VIC1_BASE : VIC2_BASE;

    int shift = interrupt < 32 ? interrupt : interrupt - 32;
    int bit = 1 << shift;

    // Invert the bits, so all except the selected bit are 1.
    bit = ~bit;

    int *disabled = (int *)(base + VIC_INT_ENABLE_OFFSET);
    *disabled &= bit;

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


