#include <clock_notifier.h>

#include <clock.h>
#include <event.h>
#include <interrupt.h>
#include <syscall.h>
#include <nameserver.h>
#include <log.h>
#include <dassert.h>

void enable_timer() {
    enable_interrupt(TIMER_3_INTERRUPT);
    int* ldr = (int *)(TIMER3_BASE + LDR_OFFSET);
    int* crtl = (int *)(TIMER3_BASE + CRTL_OFFSET);

    // Disable the clock.
    *crtl = 1 * MODE_MASK;

    // Set it's load value.
    *ldr = 2000;

    // Start the clock.
    *crtl = ENABLE_MASK + MODE_MASK;

}

void clock_notifier() {
    // Get the ClockServer TID.

    dlog("Clock Notifier: Initialized\n");

    int clock_server_tid = -2;
    do {
        clock_server_tid = WhoIs("ClockServer");
        dlog("Clock Server Tid %d\n", clock_server_tid);
    } while (clock_server_tid < 0);

    dlog("Clock Notifier: Found ClockServer\n");

    enable_timer();

    dlog("Clock Notifier: Enabled Timer\n");

    for(;;) {
        int error = AwaitEvent(TIMER_3_EVENT);
        dlog("Clock Notifier: Tick\n");
        if (error < 0) {
            dlog("Recieved an error waiting for Timer\n");
            continue;
        }

        ClockMessage msg;
        msg.type = TICK_REQUEST;
        ClockMessage rply;

        Send(clock_server_tid, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));

        dassert(rply.type == TICK_RESPONSE, "Invalid Response from Clock Server");
    }
}
