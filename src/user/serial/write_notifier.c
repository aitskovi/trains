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
    dlog("Write Notifier: Initialized\n");

    dlog("Write Notifier: Receiving Instructions\n");
    /*
    int tid = 0;
    char *reply;
    Recieve(&tid, reply, replylen);
    */
    int channel = COM1;

    int server_tid = -1;
    do {
        server_tid = WhoIs("UART1WriteServer");
        dlog("UART1WriteServer %d\n", server_tid);
    } while (server_tid < 0);

    dlog("Write Notifier: Setting up UART\n");
    uart_setspeed(COM1, 2400);
    uart_setstop(COM1, 2);
    uart_setfifo(COM1, OFF);

    enable_event(UART_1_TX_EVENT);
    enable_event(UART_1_CTS_EVENT);
    dlog("Write Notifier: Set-up UART\n");

    dlog("Write Notifier: Setting up Notifier State\n");
    if (!uart_getcts(COM1)) {
        dlog("Write Notifier: Waiting For CTS\n");
        AwaitEvent(UART_1_CTS_EVENT);
    }
    dlog("Write Notifier: Set up Notifier State\n");

    for (;;) {
        dlog("Waiting for Transmit\n");
        int error = AwaitEvent(UART_1_TX_EVENT);
        if (error < 0) {
            dlog("Recieved an error waiting for WRITE_EVENT\n");
        }

        WriteMessage msg;
        msg.type = WRITE_EVENT_REQUEST;
        WriteMessage rply;

        Send(server_tid, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));

        dassert(rply.type == WRITE_EVENT_RESPONSE, "Invalid Response from WriteServer");

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

    Exit();
}
