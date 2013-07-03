#include <sensor_courier.h>

#include <bits.h>
#include <dassert.h>
#include <log.h>
#include <nameserver.h>
#include <string.h>
#include <syscall.h>
#include <location_server.h>
#include <ts7200.h>
#include <encoding.h>

void location_courier() {

    // Find the sensor server.
    int location_server_tid = -2;
    do {
        location_server_tid = WhoIs("LocationServer");
        dlog("Location Server Tid %d\n", location_server_tid);
    } while (location_server_tid < 0);

    struct Message producer_msg, producer_rply;
    struct Message consumer_msg, consumer_rply;
    for(;;) {
        producer_msg.type = LOCATION_SERVER_MESSAGE;
        producer_msg.ls_msg.type = LOCATION_COURIER_REQUEST;
        Send(location_server_tid, (char *)&producer_msg, sizeof(producer_msg), 
                                  (char *)&producer_rply, sizeof(producer_rply));
        cuassert(producer_rply.type == LOCATION_SERVER_MESSAGE, "Invalid Response from LocationServer");
        cuassert(producer_rply.ls_msg.type == LOCATION_COURIER_RESPONSE, "Invalid Courier Message");

        // Format the message to consumers.
        consumer_msg.type = LOCATION_SERVER_MESSAGE;
        consumer_msg.ls_msg.type = LOCATION_COURIER_REQUEST;
        consumer_msg.ls_msg.train = producer_rply.ls_msg.train;
        consumer_msg.ls_msg.landmark = producer_rply.ls_msg.landmark;
        consumer_msg.ls_msg.distance = producer_rply.ls_msg.distance;
        consumer_msg.ls_msg.edge = producer_rply.ls_msg.edge;

        // Send message to all consumers.
        int i;
        for (i = 0; i < MAX_SUBSCRIBERS; ++i) {
            int subscriber = producer_rply.ls_msg.subscribers[i];
            while(subscriber) {
                int bit = ffs(subscriber) - 1;
                int tid = 32 * i + bit;

                Send(tid, (char *)&consumer_msg, sizeof(consumer_msg),
                          (char *)&consumer_rply, sizeof(consumer_rply));
                cuassert(consumer_rply.type == LOCATION_SERVER_MESSAGE, "Invalid Response from Consumer");
                cuassert(consumer_rply.ls_msg.type == LOCATION_COURIER_RESPONSE, "Invalid Response from Consumer");

                subscriber &= ~(1 << bit); // Negate the bit we used.
            }
        }
    }

    Exit();
}
