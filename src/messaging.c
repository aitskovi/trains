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

// An array of replies you're set to recieve.
static char* replies[MAX_TASKS];
static int reply_lengths[MAX_TASKS];

void initialize_messaging() {
    int i;
    for (i = 0; i < MAX_TASKS; ++i) {
        circular_queue_initialize(&mailboxes[i]);
        msgs[i] = 0;
        msg_lengths[i] = 0;
        replies[i] = 0;
        reply_lengths[i] = 0;
    }
}

int ksend(int src, int dst, char *msg, int msglen, char *reply, int replylen) {
    // Put your send buffer in place, and wait.
    msgs[src] = msg;
    msg_lengths[src] = msglen;

    // Put your reply buffer in place, and wait.
    replies[src] = reply;
    reply_lengths[src] = replylen;

    // Tell the task he has a message from you.
    int error = circular_queue_push(&mailboxes[dst], (void *)src);
    if (error) {
        bwprintf(COM2, "Error Informing Task: %d of Message from %d", dst, src);
        return -1;
    }

    return 0;
}

int krecieve(int dst, int *src, char *msg, int msglen) {
    struct circular_queue *mailbox = &mailboxes[dst];

    // Check if we have a messsage waiting for us.
    if (circular_queue_empty(mailbox)) {
        bwprintf(COM2, "No Messages!, Let's block\n");
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

int kreply(int tid, char *reply, int replylen) {
    // Ensure we're waiting for a reply, and not send blocked or not blocked at all.
    if (msgs[tid] != 0 && reply_lengths !=0) return -1;
    if (replies[tid] == 0 && reply_lengths[tid] == 0) return -2;

    // Write data into the waiting task's buffer.
    replylen = min(replylen, reply_lengths[tid]);
    memcpy(replies[tid], reply, replylen);

    replies[tid] = 0;
    reply_lengths[tid] = 0;

    return replylen;
}
