#include <messaging.h>

#include <bwio.h>
#include <circular_queue.h>
#include <memory.h>
#include <task.h>

#define min(a,b) (a) < (b) ? (a) : (b)

// A queue of messages waiting for you.
static struct circular_queue mailboxes[MAX_TASKS];

// An array of messages you've sent.
static char *msgs[MAX_TASKS];
static int msg_lengths[MAX_TASKS];

// An array of message you're set to recieve.
static char* rcvds[MAX_TASKS];
static int rcvd_lengths[MAX_TASKS];
static int* rcvd_srcs[MAX_TASKS];

void initialize_messaging() {
    int i;
    for (i = 0; i < MAX_TASKS; ++i) {
        circular_queue_initialize(&mailboxes[i]);
        msgs[i] = 0;
        msg_lengths[i] = 0;
        rcvds[i] = 0;
        rcvd_lengths[i] = 0;
        rcvd_srcs[i] = 0;
    }
}

int msg_send(int src, int dst, char *msg, int msglen, char *reply, int replylen) {
    if (msgs[src] != 0 || msg_lengths[src] != 0) return -1;
    if (rcvds[src] != 0 || rcvd_lengths[src] != 0) return -2;

    // Put your send buffer in place, and wait.
    msgs[src] = msg;
    msg_lengths[src] = msglen;

    // Put your reply buffer in place, and wait.
    rcvds[src] = reply;
    rcvd_lengths[src] = replylen;

    // Tell the task he has a message from you.
    int error = circular_queue_push(&mailboxes[dst], (void *)src);
    if (error) {
        //bwprintf(COM2, "Error Informing Task: %d of Message from %d", dst, src);
        return -3;
    }

    return 0;
}

int msg_recieve(int dst, int *src, char *msg, int msglen) {
    struct circular_queue *mailbox = &mailboxes[dst];

    if (src == 0 && msg == 0 && msglen == 0) {
        // TODO Assert that we have something in rcvds and rcvd_length
        msg = rcvds[dst];
        msglen = rcvd_lengths[dst];
        src = rcvd_srcs[dst];
        rcvds[dst] = 0;
        rcvd_lengths[dst] = 0;
        rcvd_srcs[dst] = 0;
    }

    // Assert that we don't have anything left over in our rcvds buffer.

    // Check if we have a messsage waiting for us.
    if (circular_queue_empty(mailbox)) {
        // We're about to block. Let's store our buffer.
        rcvds[dst] = msg;
        rcvd_lengths[dst] = msglen;
        rcvd_srcs[dst] = src;
        return -1;
    }

    // Grab the message.
    *src = (int)circular_queue_pop(mailbox);

    // Copy the data into the correct buffer.
    msglen = min(msglen, msg_lengths[*src]);
    memcpy(msg, msgs[*src], msglen);

    // Remove the message we've consume.
    msg_lengths[*src] = 0;
    msgs[*src] = 0;

    return msglen;
}

int msg_reply(int tid, char *reply, int replylen) {
    // Ensure we're waiting for a reply, and not send blocked or not blocked at all.
    if (msgs[tid] != 0 || msg_lengths[tid] !=0) return -1;
    if (rcvds[tid] == 0 || rcvd_lengths[tid] == 0) return -2;

    // Write data into the waiting task's buffer.
    replylen = min(replylen, rcvd_lengths[tid]);
    memcpy(rcvds[tid], reply, replylen);

    rcvds[tid] = 0;
    rcvd_lengths[tid] = 0;

    return replylen;
}
