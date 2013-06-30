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

void sensor_courier() {
    // Find the sensor server.
    int sensor_server_tid = -2;
    do {
        sensor_server_tid = WhoIs("SensorServer");
        dlog("Sensor Server Tid %d\n", sensor_server_tid);
    } while (sensor_server_tid < 0);

    SensorServerMessage msg;
    SensorServerMessage producer_rply, consumer_rply;
    msg.type = SENSOR_COURIER_REQUEST;
    for(;;) {
        Send(sensor_server_tid, (char *)&msg, sizeof(msg), (char *)&producer_rply, sizeof(producer_rply));
        dassert(producer_rply.type == SENSOR_COURIER_RESPONSE, "Invalid Response from SensorServer");

        // Format the message to consumers.
        msg.sensor = producer_rply.sensor;
        msg.number = producer_rply.number;

        // Send messageto all consumers.
        int i;
        for (i = 0; i < MAX_SUBSCRIBERS; ++i) {
            int subscriber = producer_rply.subscribers[i];
            while(subscriber) {
                int bit = ffs(subscriber) - 1;
                int tid = 32 * i + bit;

                Send(tid, (char *)&msg, sizeof(msg), (char *)&consumer_rply, sizeof(consumer_rply));
                dassert(consumer_rply.type == SENSOR_COURIER_RESPONSE, "Invalid Response from SensorServer");

                subscriber &= ~(1 << bit); // Negate the bit we used.
            }
        }
    }

    Exit();
}
