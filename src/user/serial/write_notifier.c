#include <write_notifier.h>

#include <dassert.h>
#include <event.h>
#include <log.h>
#include <nameserver.h>
#include <serial.h>
#include <syscall.h>
#include <ts7200.h>
#include <uart.h>
#include <bwio.h>

void write_notifier() {
    int server_tid, channel, event;

    dlog("Write Notifier: Waiting for Configuration\n");
    channel = get_writer_configuration(&server_tid);
    event = channel == COM1 ? UART_1_TX_EVENT : UART_2_TX_EVENT;
    dlog("Write Notifier: Configured %d\n", channel);

    dlog("Write Notifier: Initialized\n");

    dlog("Write Notifier: Setting up UART\n");
    if (channel == COM1) {
        uart_setspeed(COM1, 2400);
        uart_setstop(COM1, 2);
        enable_event(UART_1_CTS_EVENT);
    }

    uart_setfifo(channel, OFF);
    enable_event(event);
    dlog("Write Notifier: Set-up UART\n");

    // We only care about CTS for COM1.
    if (channel == COM1) {
        dlog("Write Notifier: Setting up Notifier State\n");
        if (!uart_getcts(COM1)) {
            dlog("Write Notifier: Waiting For CTS\n");
            AwaitEvent(UART_1_CTS_EVENT);
        }
        dlog("Write Notifier: Set up Notifier State\n");
    }

    for (;;) {
        dlog("Waiting for Transmit\n");
        int error = AwaitEvent(event);
        if (error < 0) {
            dlog("Recieved an error waiting for WRITE_EVENT\n");
        }

        WriteMessage msg;
        msg.type = WRITE_EVENT_REQUEST;
        WriteMessage rply;

        Send(server_tid, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));

        dassert(rply.type == WRITE_EVENT_RESPONSE, "Invalid Response from WriteServer");

        // We only care about CTS for COM1.
        if (channel == COM1) {
            // This should usually run though twice, but may run once
            // in exceptional cases.
            do {
                dlog("Waiting for CTS\n");
                dlog("CTS Pre is %d\n", uart_getcts(COM1));
                error = AwaitEvent(UART_1_CTS_EVENT);
                dlog("CTS Post is %d\n", uart_getcts(COM1));
                dassert(error >= 0, "Error waiting for CTS_EVENT\n");
            } while(!uart_getcts(COM1));
        }
    }

    Exit();
}
