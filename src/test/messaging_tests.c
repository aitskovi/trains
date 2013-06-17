// Messaging.
//
// Notes: This covers testing everything outside of scheduling.

#include <messaging.h>

#include <string.h>
#include <stdio.h>
#include <verify.h>

int messaging_basic_test() {
   int i;
   for (i = 0; i < 3; ++i) {
       int a = 0;
       int b = 1;
       char *msg = "abcdef";
       int msglen = 7;
       char reply[7];
       int replylen = 7;
       int result = msg_send(a, b, msg, msglen, reply, replylen);
       vassert(result == 0);

       int src;
       char rcvd[7];
       int rcvd_len = 7;
       result = msg_recieve(b, &src, rcvd, rcvd_len);
       vassert(src == 0);
       vassert(result == 7);
       vassert(strcmp(msg, rcvd) == 0);

       char *rply = "fedcba";
       int rply_len = 7;
       result = msg_reply(0, rply, rply_len);
       vassert(result == 7);
       vassert(strcmp(rply, reply) == 0);
   }

   return 0;
}

int messaging_send_double_test() {
    int a = 0;
    int b = 1;
    char *msg = "abcdef";
    int msglen = 7;
    char reply[7];
    int replylen = 7;
    int result = msg_send(a, b, msg, msglen, reply, replylen);
    vassert(result == 0);

    // Fail to send while recv blocked.
    result = msg_send(a, b, msg, msglen, reply, replylen);
    vassert(result == -1);

    int src;
    char rcvd[7];
    int rcvd_len = 7;
    result = msg_recieve(b, &src, rcvd, rcvd_len);
    vassert(src == 0);
    vassert(result == 7);
    vassert(strcmp(msg, rcvd) == 0);

    // Fail to send while reply blocked.
    result = msg_send(a, b, msg, msglen, reply, replylen);
    vassert(result == -2);

    return 0;
}

int messaging_recieve_blocking_test() {
    int b = 1;
    int src;
    char rcvd[7];
    int rcvd_len = 7;
    int result = msg_recieve(b, &src, rcvd, rcvd_len);
    vassert(result == -1);

    int a = 0;
    char *msg = "abcdef";
    int msglen = 7;
    char reply[7];
    int replylen = 7;
    result = msg_send(a, b, msg, msglen, reply, replylen);
    vassert(result == 0);

    // Try to recieve again. It should come in old buffers.
    result = msg_recieve(b, 0, 0, 0);
    vassert(src == 0);
    vassert(result == 7);
    vassert(strcmp(msg, rcvd) == 0);

    return 0;
}

int messaging_recieve_partial_message() {
    int a = 0;
    int b = 1;
    char *msg = "abcdef";
    int msglen = 7;
    char reply[7];
    int replylen = 7;
    int result = msg_send(a, b, msg, msglen, reply, replylen);
    vassert(result == 0);

    int src;
    char rcvd[5];
    int rcvd_len = 4;
    result = msg_recieve(b, &src, rcvd, rcvd_len);
    vassert(src == 0);
    vassert(result == 4);
    rcvd[4] = 0;
    vassert(!strcmp(rcvd, "abcd"));

    return 0;
}

int messaging_reply_non_blocked_test() {
    // Fail to reply non blocked.
    char *rply = "fedcba";
    int rply_len = 7;
    int result = msg_reply(0, rply, rply_len);
    vassert(result == -2);

    int a = 0;
    int b = 1;
    char *msg = "abcdef";
    int msglen = 7;
    char reply[7];
    int replylen = 7;
    result = msg_send(a, b, msg, msglen, reply, replylen);
    vassert(result == 0);

    // Fail to reply, recv blocked.
    result = msg_reply(0, rply, rply_len);
    vassert(result == -1);

    return 0;
}

int messaging_reply_partial_message_test() {
    int a = 0;
    int b = 1;
    char *msg = "abcdef";
    int msglen = 7;
    char reply[5];
    int replylen = 4;
    int result = msg_send(a, b, msg, msglen, reply, replylen);
    vassert(result == 0);

    int src;
    char rcvd[7];
    int rcvd_len = 7;
    result = msg_recieve(b, &src, rcvd, rcvd_len);
    vassert(src == 0);
    vassert(result == 7);
    vassert(strcmp(msg, rcvd) == 0);

    char *rply = "fedcba";
    int rply_len = 7;
    result = msg_reply(0, rply, rply_len);
    vassert(result == 4);
    reply[4] = 0;
    vassert(strcmp("fedc", reply) == 0);

    return 0;
}

int messaging_fifo_ordering_test() {
    int a = 0;
    int b = 1;
    char *msg = "abcdef";
    int msglen = 7;
    char reply[7];
    int replylen = 7;
    int result = msg_send(a, b, msg, msglen, reply, replylen);
    vassert(result == 0);

    int c = 2;
    char *msg2 = "ghijkl";
    int msglen2 = 7;
    char reply2[7];
    int replylen2 = 7;
    result = msg_send(c, b, msg2, msglen2, reply2, replylen2);
    vassert(result == 0);

    int src;
    char rcvd[7];
    int rcvd_len = 7;
    result = msg_recieve(b, &src, rcvd, rcvd_len);
    vassert(src == 0);
    vassert(result == 7);
    vassert(strcmp(msg, rcvd) == 0);

    return 0;
}

struct vsuite* messaging_suite() {
    struct vsuite *suite = vsuite_create("Messaging Tests", initialize_messaging);
    vsuite_add_test(suite, messaging_basic_test);
    vsuite_add_test(suite, messaging_send_double_test);
    vsuite_add_test(suite, messaging_recieve_blocking_test);
    vsuite_add_test(suite, messaging_recieve_partial_message);
    vsuite_add_test(suite, messaging_reply_non_blocked_test);
    vsuite_add_test(suite, messaging_reply_partial_message_test);
    return suite;
}
