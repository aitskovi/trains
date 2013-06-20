#include <read_server.h>

#include <dassert.h>
#include <log.h>
#include <nameserver.h>
#include <read_service.h>
#include <read_notifier.h>
#include <serial.h>
#include <syscall.h>
#include <task.h>
#include <ts7200.h>

static tid_t com1_read_server_tid = -1;
static tid_t com2_read_server_tid = -1;

int Getc(int channel) {
    tid_t server_tid = -1;
    switch(channel) {
        case COM1:
            server_tid = com1_read_server_tid;
            break;
        case COM2:
            server_tid = com2_read_server_tid;
            break;
        default:
            dlog("Sending to invalid Channel!\n");
    }

    if (server_tid < 0) {
        return -1;
    }

    ReadMessage msg, reply;
    msg.type = GETC_REQUEST;
    Send(server_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    dassert(reply.type == GETC_RESPONSE, "Invalid response from read server");
    return reply.data;
}

void ReadServer() {
    int tid, channel;
    ReadMessage msg;
    ReadMessage rply;
    struct ReadService service;

    dlog("Read Server: Waiting for Configuration\n");
    channel = get_reader_configuration(&tid);
    dlog("Read Server: Configured %d\n", channel);

    dlog("Read Server: Initializing\n");
    readservice_initialize(&service, channel);
    dlog("Read Server: Initialized\n");

    dlog("Read Server: Registering\n");
    if (channel == COM1) {
        RegisterAs("UART1ReadServer");
        com1_read_server_tid = MyTid();
    } else {
        RegisterAs("UART2ReadServer");
        com2_read_server_tid = MyTid();
    }

    dlog("Read Server: Creating Notifier\n");
    int read_notifier_tid = Create(REALTIME, read_notifier);

    dlog("Read Server: Configuring Notifier\n");
    configure_reader(read_notifier_tid, channel);

    dlog("Read Server: Serving Requests\n");
    for (;;) {
        Receive(&tid, (char *)&msg, sizeof(msg));

        switch(msg.type) {
            case READ_EVENT_REQUEST: {
                dlog("ReadServer: ReadEvent Request\n");

                // Reply to Notifier right away.
                rply.type = READ_EVENT_RESPONSE;
                Reply(tid, (char *)&rply, sizeof(rply));
                
                readservice_putc(&service, (char)msg.data);
                break;
            }
            case GETC_REQUEST: {
                dlog("ReadServer: Getc Request\n");
                readservice_getc(&service, tid);
                break;
            }
            default:
                dlog("Invalid ReadServer Request: %x\n", msg);
        }

        readservice_flush(&service);
    }

    dlog("Read Server: Shutting Down\n");
}
