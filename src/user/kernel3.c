#include <log.h>

#include <idle.h>
#include <interrupt.h>
#include <scheduling.h>
#include <syscall.h>
#include <ts7200.h>

void first() {
    // Setup the timer interrupt.

    log("First: Initializing\n");
    log("First: Creating Idle Task\n");
    Create(LOW, idle);
    log("First: Created Idle Task\n");

    log("First: Initializing Timer\n");

    enable_interrupt(TIMER_3_INTERRUPT);

    int* ldr = (int *)(TIMER3_BASE + LDR_OFFSET);
    int* crtl = (int *)(TIMER3_BASE + CRTL_OFFSET);

    // Disable the clock.
    *crtl = 1 * MODE_MASK;

    // Set it's load value.
    *ldr = 2000;

    // Start the clock.
    *crtl = ENABLE_MASK + MODE_MASK;

    log("First: Timer Initialized\n");

    log("First: AwaitingTimer\n");
    AwaitEvent(TIMER_3_EVENT);
    log("First: AwaitedTimer\n");

    log("First: Exiting\n");
    Exit();
}
