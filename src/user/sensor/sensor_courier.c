#include <sensor_courier.h>

#include <bits.h>
#include <dassert.h>
#include <log.h>
#include <nameserver.h>
#include <sensor_server.h>
#include <string.h>
#include <syscall.h>
#include <write_server.h>
#include <ts7200.h>
#include <encoding.h>

void sensor_courier() {
    // Find the sensor server.
    int sensor_server_tid = -2;
    do {
        sensor_server_tid = WhoIs("SensorServer");
        dlog("Sensor Server Tid %d\n", sensor_server_tid);
    } while (sensor_server_tid < 0);

    SensorServerMessage producer_msg, producer_rply;
    struct Message consumer_msg, consumer_rply;
    producer_msg.type = SENSOR_COURIER_REQUEST;
    consumer_msg.type = SENSOR_SERVER_MESSAGE;
    consumer_msg.ss_msg.type = SENSOR_COURIER_REQUEST;
    for(;;) {
        Send(sensor_server_tid, (char *)&producer_msg, sizeof(producer_msg),
                                (char *)&producer_rply, sizeof(producer_rply));
        dassert(producer_rply.type == SENSOR_COURIER_RESPONSE, "Invalid Response from SensorServer");

        // Format the message to consumers.
        consumer_msg.ss_msg.sensor = producer_rply.sensor;
        consumer_msg.ss_msg.number = producer_rply.number;

        // Send message to all consumers.
        int i;
        for (i = 0; i < MAX_SUBSCRIBERS; ++i) {
            int subscriber = producer_rply.subscribers[i];
            while(subscriber) {
                int bit = ffs(subscriber) - 1;
                int tid = 32 * i + bit;

                Send(tid, (char *)&consumer_msg, sizeof(consumer_msg),
                          (char *)&consumer_rply, sizeof(consumer_rply));
                dassert(consumer_rply.type == SENSOR_SERVER_MESSAGE, "Invalid Response from Consumer");
                dassert(consumer_rply.ss_msg.type == SENSOR_COURIER_RESPONSE, "Invalid Response from Consumer");

                subscriber &= ~(1 << bit); // Negate the bit we used.
            }
        }
    }

    Exit();
}
