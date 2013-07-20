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

int Write(int channel, char *str, unsigned int size) {
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
            break;
    }

    if (server_tid < 0) {
        return -1;
    }
    WriteMessage msg, reply;
    msg.type = WRITE_REQUEST;
    msg.data = str;
    msg.length = size;
    Send(server_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    dassert(reply.type == WRITE_RESPONSE, "Invalid response from write server");
    return 0;
}

int Putc(int channel, char ch) {
    return Write(channel, &ch, 1);
}

void WriteServer() {
    int tid, channel;
    WriteMessage msg;
    WriteMessage rply;
    struct WriteService service;

    dlog("Write Server: Waiting for Configuration\n");
    channel = get_writer_configuration(&tid);
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
    configure_writer(write_notifier_tid, channel);
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
            case WRITE_REQUEST: {
                writeservice_enqueue(&service, msg.data, msg.length);

                rply.type = WRITE_RESPONSE;
                Reply(tid, (char *)&rply, sizeof(rply));
                break;
            }
            default:
                dlog("Invalid WriteServer Request: %x\n", msg);
                break;
        }

        writeservice_flush(&service);
    }

    dlog("Write Server: Shutting Down\n");
}
