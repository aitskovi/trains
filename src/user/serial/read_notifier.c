#include <read_notifier.h>

#include <dassert.h>
#include <event.h>
#include <log.h>
#include <nameserver.h>
#include <serial.h>
#include <syscall.h>
#include <ts7200.h>
#include <uart.h>

void read_notifier() {
    dlog("Read Notifier: Initialized\n");

    dlog("Read Notifier: Receiving Instructions\n");
    /*
    int tid = 0;
    char *reply;
    Recieve(&tid, reply, replylen);
    */
    int channel = COM1;

    int server_tid = -1;
    do {
        server_tid = WhoIs("UART1ReadServer");
        dlog("UART1ReadServer %d\n", server_tid);
    } while (server_tid < 0);

    dlog("Read Notifier: Setting up UART\n");
    uart_setspeed(COM1, 2400);
    uart_setstop(COM1, 2);
    uart_setfifo(COM1, OFF);

    enable_event(UART_1_RCV_EVENT);
    dlog("Read Notifier: Set-up UART\n");

    dlog("Read Notifier: Setting up Notifier State\n");
    dlog("Read Notifier: Set up Notifier State\n");

    for (;;) {
        dlog("Read Notifier: Waiting for RCV Event\n");
        int data = AwaitEvent(UART_1_RCV_EVENT);
        if (data < 0) {
            dlog("Recieved an error waiting for READ_EVENT\n");
        }

        ReadMessage msg;
        msg.type = READ_EVENT_REQUEST;
        msg.data = data;
        ReadMessage rply;
        Send(server_tid, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
        dassert(rply.type == READ_EVENT_RESPONSE, "Invalid Response from ReadServer");
    }

    Exit();
}
