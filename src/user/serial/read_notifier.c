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
    int server_tid, channel, event;
    ReadMessage msg, rply;

    dlog("Read Notifier: Waiting for Configuration\n");
    Receive(&server_tid, (char *)&msg, sizeof(msg));
    dassert(msg.type == READ_CONFIG_REQUEST, "Invalid Config Message");
    rply.type = READ_CONFIG_RESPONSE;
    Reply(server_tid, (char *)&rply, sizeof(rply));
    channel = msg.data;
    event = channel == COM1 ? UART_1_RCV_EVENT : UART_2_RCV_EVENT;
    dlog("Read Notifier: Configured %d\n", channel);

    dlog("Read Notifier: Setting up UART\n");
    if (channel == COM1) {
        uart_setspeed(COM1, 2400);
        uart_setstop(COM1, 2);
    }

    uart_setfifo(channel, OFF);
    enable_event(event);
    dlog("Read Notifier: Set-up UART\n");

    for (;;) {
        dlog("Read Notifier: Waiting for RCV Event\n");
        int data = AwaitEvent(event);
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
