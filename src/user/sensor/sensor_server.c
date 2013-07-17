/*
 * sensor_server.c
 *
 *  Created on: Jun 22, 2013
 *      Author: aianus
 */

#include <sensor_server.h>

#include <encoding.h>
#include <dassert.h>
#include <sprintf.h>
#include <read_server.h>
#include <write_server.h>
#include <ts7200.h>
#include <memory.h>
#include <scheduling.h>
#include <nameserver.h>
#include <syscall.h>
#include <sensor_notifier.h>
#include <sensor_service.h>
#include <task.h>
#include <pubsub.h>

/**
 * Send a notification message through the courier.
 */
static void publish(struct SensorService *service, int tid) {
    while(1) {
        Message msg;
        msg.type = SENSOR_SERVER_MESSAGE;
        msg.ss_msg.type = SENSOR_COURIER_REQUEST;

        int result = sensorservice_pop(service, &(msg.ss_msg.sensor), &(msg.ss_msg.number), msg.subscribers);
        if (result == -1) break;

        Publish(tid, &msg);
    }
}

void sensor_server() {
    struct SensorService service;    
    sensorservice_initialize(&service);

    RegisterAs("SensorServer");

    // Create the thing sending us sensor messages.
    Create(HIGH, sensor_notifier);

    tid_t stream = CreateStream("SensorServerStream");

    int tid;
    SensorServerMessage msg, rply;
    for(;;) {
        Receive(&tid, (char *) &msg, sizeof(msg));

        switch(msg.type) {
            case SENSOR_EVENT_REQUEST:
                rply.type = SENSOR_EVENT_RESPONSE;
                Reply(tid, (char *) &rply, sizeof(rply));

                sensorservice_process_data(&service, msg.data);

                publish(&service, stream);
                break;
            default:
                ulog("Invalid SensorServer Request");
        }
    }

    Exit();
}
