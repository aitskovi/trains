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
    dlog("Write Server: Initialized\n");
    struct WriteService service;
    writeservice_initialize(&service, COM1);

    dlog("Write Server: Registering\n");
    RegisterAs("UART1WriteServer");
    com1_server_tid = MyTid();

    dlog("Write Server: Creating Notifier\n");
    Create(REALTIME, write_notifier);

    dlog("Write Server: Serving Requests\n");

    int tid;
    WriteMessage msg;
    WriteMessage rply;
    for (;;) {
        Receive(&tid, (char *)&msg, sizeof(msg));

        switch(msg.type) {
            case WRITE_EVENT_REQUEST: {
                // Reply to Notifier right away.
                WriteMessage rply;
                rply.type = WRITE_EVENT_RESPONSE;
                Reply(tid, (char *)&rply, sizeof(rply));
                
                writeservice_writable(&service);
                break;
            }
            case PUTC_REQUEST: {
                WriteMessage rply;
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
