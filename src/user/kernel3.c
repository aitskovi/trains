#include <syscall.h>
#include <ts7200.h>
#include <log.h>

#include <interrupt.h>

void first() {
    // Setup the timer interrupt.

    enable_interrupt(TIMER_3_INTERRUPT);

    int i;
    for (i = 0; i < 3; ++i) {
        log("Generating Interrupt\n");
        generate_interrupt(TIMER_3_INTERRUPT);
        log("Generated Interrupt\n");
    }

    Exit();
}
