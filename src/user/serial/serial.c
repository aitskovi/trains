#include <serial.h>

#include <dassert.h>
#include <log.h>
#include <syscall.h>

void configure_writer(int tid, int channel) {
    WriteMessage wmsg, wrply;
    wmsg.type = WRITE_CONFIG_REQUEST;
    wmsg.data = channel;
    Send(tid, (char *)&wmsg, sizeof(wmsg), (char *)&wrply, sizeof(wrply));
    dassert(wrply.type == WRITE_CONFIG_RESPONSE, "Invalid Config Reply");
}

void configure_reader(int tid, int channel) {
    ReadMessage rmsg, rrply;
    rmsg.type = READ_CONFIG_REQUEST;
    rmsg.data = channel;
    Send(tid, (char *)&rmsg, sizeof(rmsg), (char *)&rrply, sizeof(rrply));
    dassert(rrply.type == READ_CONFIG_RESPONSE, "Invalid Config Reply");
}

int get_reader_configuration(int *tid) {
    ReadMessage msg, rply;
    Receive(tid, (char *)&msg, sizeof(msg));
    dassert(msg.type == READ_CONFIG_REQUEST, "Invalid Config Message");
    rply.type = READ_CONFIG_RESPONSE;
    Reply(*tid, (char *)&rply, sizeof(rply));
    return msg.data;
}

int get_writer_configuration(int *tid) {
    WriteMessage msg, rply;
    Receive(tid, (char *)&msg, sizeof(msg));
    dassert(msg.type == WRITE_CONFIG_REQUEST, "Invalid Config Message");
    rply.type = WRITE_CONFIG_RESPONSE;
    Reply(*tid, (char *)&rply, sizeof(rply));
    return msg.data;
}

