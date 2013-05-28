// Messaging.
//
// Notes: This covers testing everything outside of scheduling.

#include <messaging.h>

#include <string.h>
#include <assert.h>
#include <stdio.h>

void messaging_basic_test() {
   initialize_messaging();

   int i;
   for (i = 0; i < 3; ++i) {
       int a = 0;
       int b = 1;
       char *msg = "abcdef";
       int msglen = 7;
       char reply[7];
       int replylen = 7;
       int result = msg_send(a, b, msg, msglen, reply, replylen);
       assert(result == 0);

       int src;
       char rcvd[7];
       int rcvd_len = 7;
       result = msg_recieve(b, &src, rcvd, rcvd_len);
       assert(src == 0);
       assert(result == 7);
       assert(strcmp(msg, rcvd) == 0);

       char *rply = "fedcba";
       int rply_len = 7;
       result = msg_reply(0, rply, rply_len);
       assert(result == 7);
       assert(strcmp(rply, reply) == 0);
   }
}

void messaging_send_double_test() {
    initialize_messaging();

    int a = 0;
    int b = 1;
    char *msg = "abcdef";
    int msglen = 7;
    char reply[7];
    int replylen = 7;
    int result = msg_send(a, b, msg, msglen, reply, replylen);
    assert(result == 0);

    // Fail to send while recv blocked.
    result = msg_send(a, b, msg, msglen, reply, replylen);
    assert(result == -1);

    int src;
    char rcvd[7];
    int rcvd_len = 7;
    result = msg_recieve(b, &src, rcvd, rcvd_len);
    assert(src == 0);
    assert(result == 7);
    assert(strcmp(msg, rcvd) == 0);

    // Fail to send while reply blocked.
    result = msg_send(a, b, msg, msglen, reply, replylen);
    assert(result == -2);
}

void messaging_recieve_blocking_test() {
    initialize_messaging();

    int b = 1;
    int src;
    char rcvd[7];
    int rcvd_len = 7;
    int result = msg_recieve(b, &src, rcvd, rcvd_len);
    assert(result == -1);

    int a = 0;
    char *msg = "abcdef";
    int msglen = 7;
    char reply[7];
    int replylen = 7;
    result = msg_send(a, b, msg, msglen, reply, replylen);
    assert(result == 0);

    // Try to recieve again. It should come in old buffers.
    result = msg_recieve(b, 0, 0, 0);
    assert(src == 0);
    assert(result == 7);
    assert(strcmp(msg, rcvd) == 0);

}

void messaging_recieve_partial_message() {
    initialize_messaging();

    int a = 0;
    int b = 1;
    char *msg = "abcdef";
    int msglen = 7;
    char reply[7];
    int replylen = 7;
    int result = msg_send(a, b, msg, msglen, reply, replylen);
    assert(result == 0);

    int src;
    char rcvd[5];
    int rcvd_len = 4;
    result = msg_recieve(b, &src, rcvd, rcvd_len);
    assert(src == 0);
    assert(result == 4);
    rcvd[5] = 0;
    assert(strcmp(rcvd, "abcd"));
}

void messaging_reply_non_blocked_test() {
    initialize_messaging();

    // Fail to reply non blocked.
    char *rply = "fedcba";
    int rply_len = 7;
    int result = msg_reply(0, rply, rply_len);
    assert(result == -2);

    int a = 0;
    int b = 1;
    char *msg = "abcdef";
    int msglen = 7;
    char reply[7];
    int replylen = 7;
    result = msg_send(a, b, msg, msglen, reply, replylen);
    assert(result == 0);

    // Fail to reply, recv blocked.
    result = msg_reply(0, rply, rply_len);
    assert(result == -1);
}

void messaging_reply_partial_message_test() {
    initialize_messaging();

    int a = 0;
    int b = 1;
    char *msg = "abcdef";
    int msglen = 7;
    char reply[5];
    int replylen = 4;
    int result = msg_send(a, b, msg, msglen, reply, replylen);
    assert(result == 0);

    int src;
    char rcvd[7];
    int rcvd_len = 7;
    result = msg_recieve(b, &src, rcvd, rcvd_len);
    assert(src == 0);
    assert(result == 7);
    assert(strcmp(msg, rcvd) == 0);

    char *rply = "fedcba";
    int rply_len = 7;
    result = msg_reply(0, rply, rply_len);
    assert(result == 4);
    reply[5] = 0;
    assert(strcmp("fedc", reply) == 0);
}

void messaging_fifo_ordering_test() {
    initialize_messaging();

    int a = 0;
    int b = 1;
    char *msg = "abcdef";
    int msglen = 7;
    char reply[7];
    int replylen = 7;
    int result = msg_send(a, b, msg, msglen, reply, replylen);
    assert(result == 0);

    int c = 2;
    char *msg2 = "ghijkl";
    int msglen2 = 7;
    char reply2[7];
    int replylen2 = 7;
    result = msg_send(c, b, msg2, msglen2, reply2, replylen2);
    assert(result == 0);

    int src;
    char rcvd[7];
    int rcvd_len = 7;
    result = msg_recieve(b, &src, rcvd, rcvd_len);
    assert(src == 0);
    assert(result == 7);
    assert(strcmp(msg, rcvd) == 0);
}

int main() {
    messaging_basic_test();
    messaging_send_double_test();
    messaging_recieve_blocking_test();
    messaging_recieve_partial_message();
    messaging_reply_non_blocked_test();
    messaging_reply_partial_message_test();

    return 0;
}
