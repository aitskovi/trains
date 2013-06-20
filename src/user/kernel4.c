#include <log.h>

#include <clock_server.h>
#include <dassert.h>
#include <event.h>
#include <idle.h>
#include <interrupt.h>
#include <nameserver.h>
#include <read_server.h>
#include <scheduling.h>
#include <syscall.h>
#include <serial.h>
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

    Putc(COM1, 133);
    int i;
    for (i = 0; i < 10; ++i) {
        char a = (char)Getc(COM1);
        log("Received: %x\n", a);
    }

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
    int write_server_tid = Create(HIGHEST, WriteServer);
    log("First: Configuring WriteServer\n");
    WriteMessage msg, rply;
    msg.type = WRITE_CONFIG_REQUEST;
    msg.data = COM1;
    Send(write_server_tid, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
    dassert(rply.type == WRITE_CONFIG_RESPONSE, "Invalid Config Reply");

    log("First: Creating ReadServer\n");
    Create(HIGHEST, ReadServer);

    int writer_tid = Create(MEDIUM, writer);

    // Wait for all children to exit.
    WaitTid(writer_tid);

    log("First: Exiting\n");

    Shutdown();
}
