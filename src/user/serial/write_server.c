#include <write_server.h>

#include <dassert.h>
#include <nameserver.h>
#include <serial.h>
#include <syscall.h>
#include <task.h>
#include <ts7200.h>
#include <write_service.h>
#include <write_notifier.h>

static tid_t com1_server_tid = -1;
static tid_t com2_server_tid = -1;

int Putc(int channel, char ch) {
    tid_t server_tid = -1;
    switch(channel) {
        case COM1:
            server_tid = com1_server_tid;
            break;
        case COM2:
            server_tid = com2_server_tid;
            break;
        default:
            dlog("Sending to invalid Channel!\n");
    }

    if (server_tid < 0) {
        return -1;
    }
    WriteMessage msg, reply;
    msg.type = PUTC_REQUEST;
    msg.data = (int)ch;
    Send(server_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    dassert(reply.type == PUTC_RESPONSE, "Invalid response from write server");
    return 0;
}

void WriteServer() {
    int tid, channel;
    WriteMessage msg;
    WriteMessage rply;
    struct WriteService service;

    dlog("Write Server: Waiting for Configuration\n");
    Receive(&tid, (char *)&msg, sizeof(msg));
    dassert(msg.type == WRITE_CONFIG_REQUEST, "Invalid Config Message");
    rply.type = WRITE_CONFIG_RESPONSE;
    Reply(tid, (char *)&rply, sizeof(rply));
    channel = msg.data;
    dlog("Write Server: Configured %d\n", channel);

    dlog("Write Server: Initializing\n");
    writeservice_initialize(&service, channel);
    dlog("Write Server: Initialized\n");

    dlog("Write Server: Registering\n");
    if (channel == COM1) {
        RegisterAs("UART1WriteServer");
        com1_server_tid = MyTid();
    } else {
        RegisterAs("UART2WriteServer");
        com2_server_tid = MyTid();
    }
    dlog("Write Server: Registered\n");

    dlog("Write Server: Creating Notifier\n");
    int write_notifier_tid = Create(REALTIME, write_notifier);
    msg.type = WRITE_CONFIG_REQUEST;
    msg.data = channel;
    Send(write_notifier_tid, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
    dassert(rply.type == WRITE_CONFIG_RESPONSE, "Invalid Config Reply");
    dlog("Write Server: Created Notifier\n");

    dlog("Write Server: Serving Requests\n");
    for (;;) {
        Receive(&tid, (char *)&msg, sizeof(msg));

        switch(msg.type) {
            case WRITE_EVENT_REQUEST: {
                // Reply to Notifier right away.
                rply.type = WRITE_EVENT_RESPONSE;
                Reply(tid, (char *)&rply, sizeof(rply));
                
                writeservice_writable(&service);
                break;
            }
            case PUTC_REQUEST: {
                rply.type = PUTC_RESPONSE;
                Reply(tid, (char *)&rply, sizeof(rply));

                writeservice_enqueue(&service, (char)msg.data);
                break;
            }
            default:
                dlog("Invalid WriteServer Request: %x\n", msg);
        }

        writeservice_flush(&service);
    }

    dlog("Write Server: Shutting Down\n");
}
