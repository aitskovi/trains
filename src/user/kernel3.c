#include <log.h>

#include <idle.h>
#include <scheduling.h>
#include <syscall.h>
#include <clock_server.h>

void first() {
    // Setup the timer interrupt.

    log("First: Initializing\n");
    log("First: Creating Idle Task\n");
    Create(LOW, idle);
    log("First: Created Idle Task\n");

    log("First: Creating ClockServer\n");
    Create(HIGH, clock_server);

    log("First: Exiting\n");
    Exit();
}
