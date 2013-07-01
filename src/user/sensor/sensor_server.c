/*
 * sensor_server.c
 *
 *  Created on: Jun 22, 2013
 *      Author: aianus
 */

#include <sensor_server.h>

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
#include <sensor_courier.h>
#include <task.h>

/**
 * Send a notification message through the courier.
 */
int publish(struct SensorService *service, int tid) {
    SensorServerMessage rply;
    rply.type = SENSOR_COURIER_RESPONSE;

    int result = sensorservice_pop(service, &(rply.sensor), &(rply.number), rply.subscribers);
    if (result == -1) return -1;

    Reply(tid, (char *) &rply, sizeof(rply));

    return 0;
}

void sensor_server() {
    tid_t courier = -1;
    struct SensorService service;    
    sensorservice_initialize(&service);

    RegisterAs("SensorServer");

    // Create the thing sending us sensor messages.
    Create(HIGH, sensor_notifier);

    // Create the thing distributing our sensor data.
    Create(HIGH, sensor_courier);

    int tid;
    SensorServerMessage msg, rply;
    for(;;) {
        Receive(&tid, (char *) &msg, sizeof(msg));

        switch(msg.type) {
            case SENSOR_EVENT_REQUEST:
                rply.type = SENSOR_EVENT_RESPONSE;
                Reply(tid, (char *) &rply, sizeof(rply));

                sensorservice_process_data(&service, msg.data);

                // Wake-up a a waiting courier if we have one.
                if (courier >= 0) {
                    if (publish(&service, courier) != -1) {
                        courier = -1;
                    }
                }
                break;
            case SENSOR_COURIER_REQUEST: {
                int result = publish(&service, tid);
                if (result == -1) {
                    courier = tid;
                }
                break;
            }
            case SENSOR_SUBSCRIBE_REQUEST:
                rply.type = SENSOR_SUBSCRIBE_RESPONSE;
                Reply(tid, (char *) &rply, sizeof(rply));
                sensorservice_subscribe(&service, tid);
                break;
            default:
                ulog("Invalid SensorServer Request");
        }
    }

    Exit();
}


void sensor_server_subscribe(int server) {
    SensorServerMessage msg, rply;
    msg.type = SENSOR_SUBSCRIBE_REQUEST;
    Send(server, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
    dassert(rply.type == SENSOR_SUBSCRIBE_RESPONSE, "Invalid Response from SensorServer");
}
