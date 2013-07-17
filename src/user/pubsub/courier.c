#include <pubsub.h>

#include <bits.h>
#include <encoding.h>
#include <syscall.h>

void courier(enum PUBSUB_PRIORITY priority) {
    
    // Our parent is the pubsub server.
    tid_t producer = MyParentTid();

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

        int i;
        for (i = 0; i < MAX_SUBSCRIBERS; ++i) {
            int subscriber = producer_rply.subscribers[i];
            while(subscriber) {
                int bit = ffs(subscriber) - 1;
                int tid = 32 * i + bit;

                Send(tid, (char *)&consumer_msg, sizeof(consumer_msg),
                          (char *)&consumer_rply, sizeof(consumer_rply));

                subscriber &= ~(1 << bit); // Negate the bit we used.
            }
        }
    }

    Exit();
}
