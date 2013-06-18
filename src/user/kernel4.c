#include <log.h>

#include <clock_server.h>
#include <dassert.h>
#include <event.h>
#include <idle.h>
#include <interrupt.h>
#include <nameserver.h>
#include <scheduling.h>
#include <syscall.h>
#include <write_server.h>
#include <bwio.h>
#include <ts7200.h>
#include <uart.h>

void train_set_speed(int train, int speed) {
    /*
    uart_setspeed(COM1, 2400);
    uart_setstop(COM1, 2);
    uart_setfifo(COM1, OFF);
    uart_setfifo(COM2, OFF);
    */

    Putc(COM1, (char)speed);
    Putc(COM1, (char)train);
}

void writer() {
    //log("Writer: Initializing\n");

    train_set_speed(47, 8);
    /*
    Putc(COM2, 'H');
    Putc(COM2, 'e');
    Putc(COM2, 'l');
    Putc(COM2, 'l');
    Putc(COM2, 'o');
    Putc(COM2, '\n');
    */

    Exit();
}

void first() {
    // Setup the timer.
    log("First: Initializing\n");
    log("First: Creating Idle Task\n");
    Create(LOWEST, idle);
    log("First: Created Idle Task\n");

    // Setup the NameServer
    log("First: Creating NameServer\n");
    Create(REALTIME, NameServer);

    // Setup the clockerver.
    log("First: Creating ClockServer\n");
    Create(HIGHEST, clock_server);

    // Write some shit.
    log("First: Creating WriteServer\n");
    Create(HIGHEST, WriteServer);

    int writer_tid = Create(MEDIUM, writer);

    // Wait for all children to exit.
    WaitTid(writer_tid);

    log("First: Exiting\n");

    Shutdown();
}
