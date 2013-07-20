#include <pubsub.h>

#include <dassert.h>
#include <bits.h>
#include <encoding.h>
#include <syscall.h>

void courier(enum PUBSUB_PRIORITY priority) {
    
    // Our parent is the pubsub server.
    tid_t producer = MyParentTid();

    int logging = 0;
    tid_t me = MyTid();

    Message producer_msg, producer_rply;
    Message consumer_msg, consumer_rply;
    for(;;) {
        producer_msg.type = PUBSUB_MESSAGE;
        producer_msg.ps_msg.type = COURIER;
        producer_msg.ps_msg.priority = priority;
        Send(producer, (char *)&producer_msg, sizeof(producer_msg), 
                       (char *)&producer_rply, sizeof(producer_rply));

        // Send message to all consumers.
        consumer_msg = producer_rply;

        if (priority == 0 && consumer_msg.type == RESERVATION_SERVER_MESSAGE) {
            logging = 1;
        } else {
            logging = 0;
        }

        int i;
        for (i = 0; i < MAX_SUBSCRIBERS; ++i) {
            int subscriber = producer_rply.subscribers[i];
            while(subscriber) {
                int bit = ffs(subscriber) - 1;
                int tid = 32 * i + bit;

                //if (logging) ulog("Sending %s to %d from %d", consumer_msg.rs_msg.node->name, tid, me);
                int error = Send(tid, (char *)&consumer_msg, sizeof(consumer_msg),
                          (char *)&consumer_rply, sizeof(consumer_rply));
                //if (logging) ulog("Sent %s to %d from %d", consumer_msg.rs_msg.node->name, tid, me);
                cuassert(error > 0, "Error publishing to consumer");
                cuassert(consumer_rply.type == consumer_msg.type, "Received Invalid Reply");

                subscriber &= ~(1 << bit); // Negate the bit we used.
            }
        }
    }

    Exit();
}
