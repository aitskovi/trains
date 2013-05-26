#include <messaging.h>

#include <bwio.h>
#include <circular_queue.h>
#include <memory.h>
#include <task.h>

#define min(a,b) (a) < (b) ? (a) : (b)

struct message_queue {
    struct circular_queue src_queue;    /* The src tid that sent you the message */
    struct circular_queue data_queue;   /* The pieces of data */
    struct circular_queue len_queue;    /* The lengths */
};

int message_queue_empty(struct message_queue *queue) {
    int empty_src = circular_queue_empty(&(queue->src_queue));
    int empty_data = circular_queue_empty(&(queue->data_queue));
    int empty_len = circular_queue_empty(&(queue->len_queue));

    // TODO: Check that all empties are the same.

    return empty_src && empty_data && empty_len;
}

int message_queue_pop(struct message_queue *queue, int *src, char **data, int *len) {
    *src = (int)circular_queue_pop(&(queue->src_queue));
    *data = (char*)circular_queue_pop(&(queue->data_queue));
    *len = (int)circular_queue_pop(&(queue->len_queue));

    return 0;
}

int message_queue_push(struct message_queue *queue, int src, char *data, int len) {
    circular_queue_push(&(queue->src_queue), (void *)src);
    circular_queue_push(&(queue->data_queue), (void *)data);
    circular_queue_push(&(queue->len_queue), (void *)len);

    return 0;
}

// A circular queue of messages you're recieving.
static struct message_queue message_queues[MAX_TASKS];

// An array of replies you're set to recieve.
static char* replies[MAX_TASKS];
static int reply_lengths[MAX_TASKS];

int ksend(int src_tid, int dst_tid, char *msg, int msglen, char *reply, int replylen) {
    //TODO(Add Valid TID Check)
    //TODO(Add Running Task Check)

    int error = message_queue_push(&(message_queues[dst_tid]), src_tid, msg, msglen);
    if (error) {
        bwprintf(COM2, 
                "Placing a message onto the message queue for task: %d, failed",
                dst_tid);
    }


    replies[src_tid] = reply;
    reply_lengths[src_tid] = replylen;

    // TODO Attempt to unblock tasks.

    return 0;
}

int krecieve(int dst_tid, int *src_tid, char *msg, int msglen) {
    //TODO(Add Valid TID check for dst_tid)
    struct message_queue *mqueue = &(message_queues[dst_tid]);

    if (message_queue_empty(mqueue)) {
        // BLOCK
        bwprintf(COM2, "No Messages!, Let's block\n");
        return -1;
    }

    // Grab the message.
    char *data;
    int len;
    int error = (int)message_queue_pop(mqueue, src_tid, &data, &len);
    if (error) {
        bwprintf(COM2, "Error popping message of queue");
        return -2;
    }

    // Copy the data into the correct buffer.
    msglen = min(msglen, len);
    memcpy(msg, data, msglen);

    return msglen;
}
