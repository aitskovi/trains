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
#include <shell.h>
#include <mission_control.h>
#include <location_server.h>
#include <sensor_server.h>
#include <calibration.h>

void first() {
    // Setup the timer.
    dlog("First: Initializing\n");
    dlog("First: Creating Idle Task\n");
    Create(LOWEST, idle);
    dlog("First: Created Idle Task\n");

    // Setup the NameServer
    dlog("First: Creating NameServer\n");
    Create(HIGHEST, NameServer);

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

    Create(HIGHER, mission_control);
    Create(HIGH, sensor_server);
    Create(HIGH, LocationServer);
    Create(MEDIUM, calibration_server);

    int shell_tid = Create(MEDIUM, shell);

    //Create(HIGH, LocationServer);
    // Wait for all children to exit.
    WaitTid(shell_tid);

    dlog("First: Exiting\n");

    Shutdown();
}
