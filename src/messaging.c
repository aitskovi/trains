#include <messaging.h>

#include <bwio.h>
#include <circular_queue.h>
#include <memory.h>
#include <task.h>

#define min(a,b) (a) < (b) ? (a) : (b)

// A queue of tids that have sent messages a to given thread
static struct circular_queue mailboxes[MAX_TASKS];

// Buffers used for sending
static char *send_buffers[MAX_TASKS];
static int send_buffer_lengths[MAX_TASKS];

// Buffers used for receiving
static char* receive_buffers[MAX_TASKS];
static int receive_buffer_lengths[MAX_TASKS];
static int* receive_buffer_sources[MAX_TASKS];

void initialize_messaging() {
    int i;
    for (i = 0; i < MAX_TASKS; ++i) {
        circular_queue_initialize(&mailboxes[i]);
        send_buffers[i] = 0;
        send_buffer_lengths[i] = 0;
        receive_buffers[i] = 0;
        receive_buffer_lengths[i] = 0;
        receive_buffer_sources[i] = 0;
    }
}

int msg_send(int src, int dst, char *msg, int msglen, char *reply, int replylen) {
    if (send_buffers[src] != 0 || send_buffer_lengths[src] != 0) return -1;
    if (receive_buffers[src] != 0 || receive_buffer_lengths[src] != 0) return -2;

    // Put your send buffer in place, and wait.
    send_buffers[src] = msg;
    send_buffer_lengths[src] = msglen;

    // Put your reply buffer in place, and wait.
    receive_buffers[src] = reply;
    receive_buffer_lengths[src] = replylen;

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
        msg = receive_buffers[dst];
        msglen = receive_buffer_lengths[dst];
        src = receive_buffer_sources[dst];
        receive_buffers[dst] = 0;
        receive_buffer_lengths[dst] = 0;
        receive_buffer_sources[dst] = 0;
    }

    // Assert that we don't have anything left over in our rcvds buffer.

    // Check if we have a message waiting for us.
    if (circular_queue_empty(mailbox)) {
        // We're about to block. Let's store our buffer.
        receive_buffers[dst] = msg;
        receive_buffer_lengths[dst] = msglen;
        receive_buffer_sources[dst] = src;
        return -1;
    }

    // Grab the message.
    *src = (int)circular_queue_pop(mailbox);

    // Copy the data into the correct buffer.
    msglen = min(msglen, send_buffer_lengths[*src]);
    memcpy(msg, send_buffers[*src], msglen);

    // Remove the message we've consume.
    send_buffer_lengths[*src] = 0;
    send_buffers[*src] = 0;

    return msglen;
}

int msg_reply(int tid, char *reply, int replylen) {
    // Ensure we're waiting for a reply, and not send blocked or not blocked at all.
    if (send_buffers[tid] != 0 || send_buffer_lengths[tid] !=0) return -1;
    if (receive_buffers[tid] == 0 || receive_buffer_lengths[tid] == 0) return -2;

    // Write data into the waiting task's buffer.
    replylen = min(replylen, receive_buffer_lengths[tid]);
    memcpy(receive_buffers[tid], reply, replylen);

    receive_buffers[tid] = 0;
    receive_buffer_lengths[tid] = 0;

    return replylen;
}
