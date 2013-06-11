#include <user.h>

#include <bwio.h>
#include <syscall.h>
#include <task.h>
#include <nameserver.h>
#include <circular_queue.h>

void registration() {
    bwprintf(COM2, "Registration: Starting\n");
    bwprintf(COM2, "Registration: Registering as Driver\n");
    RegisterAs("Driver");
    bwprintf(COM2, "Registration: Registered as Driver\n");

    bwprintf(COM2, "Registration: Looking up Driver\n");
    int tid = WhoIs("Driver");
    bwprintf(COM2, "Registration: Driver is %d\n", tid);

    bwprintf(COM2, "Registration: Exiting\n");
    Exit();
}
