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

    /*
    Putc(COM1, 133);
    int i;
    for (i = 0; i < 10; ++i) {
        char a = (char)Getc(COM1);
        dlog("Received: %x\n", a);
    }
    */
    for (;;) {
        char c = (char)Getc(COM2);
        nbputc(COM2, c);
        if (c == 'q') break;
    }

    nbprintf(COM2, "Hello\n");

    Exit();
}

void first() {
    // Setup the timer.
    dlog("First: Initializing\n");
    dlog("First: Creating Idle Task\n");
    Create(LOWEST, idle);
    dlog("First: Created Idle Task\n");

    // Setup the NameServer
    dlog("First: Creating NameServer\n");
    Create(REALTIME, NameServer);

    // Setup the clockerver.
    dlog("First: Creating ClockServer\n");
    Create(HIGHEST, clock_server);

    // Write some shit.
    dlog("First: Creating WriteServer\n");
    int write_server_tid_1 = Create(HIGHEST, WriteServer);
    int write_server_tid_2 = Create(HIGHEST, WriteServer);
    dlog("First: Configuring WriteServer\n");
    configure_writer(write_server_tid_1, COM1);
    configure_writer(write_server_tid_2, COM2);

    dlog("First: Creating ReadServer\n");
    int read_server_tid_1 = Create(HIGHEST, ReadServer);
    int read_server_tid_2 = Create(HIGHEST, ReadServer);
    dlog("First: Configuring ReadServer\n");
    configure_reader(read_server_tid_1, COM1);
    configure_reader(read_server_tid_2, COM2);

    int writer_tid = Create(MEDIUM, writer);

    // Wait for all children to exit.
    WaitTid(writer_tid);

    dlog("First: Exiting\n");

    Shutdown();
}
