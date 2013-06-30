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

typedef int bool;

#define true 1
#define false 0

static char triggered_sensor[NUM_READINGS];
static int triggered_number[NUM_READINGS];

void process_sensors(char *data);

void sensor_list_print() {

    char command[512];
    char *pos = &command[0];

    pos += sprintf(pos, "\0337\033[%u;%uH\033[K", SENSOR_LIST_HEIGHT, 1);
    pos += sprintf(pos, "Recently Triggered:");

    int i;
    for (i = 0; i < NUM_READINGS; ++i) {
        if (triggered_number[i] == 0) break;
        pos += sprintf(pos, " %c%d ", triggered_sensor[i], triggered_number[i]);
    }

    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
}

void sensor_list_add(char sensor, int number) {
    // Dedupe, we get a lot of fast triggers for same sensor.
    if (triggered_sensor[0] == sensor && triggered_number[0] == number) return;

    int i;
    for (i = NUM_READINGS - 1; i > 0; --i) {
        triggered_sensor[i] = triggered_sensor[i - 1];
        triggered_number[i] = triggered_number[i - 1];
    }
    triggered_sensor[0] = sensor;
    triggered_number[0] = number;
}

void sensors_init() {
    memset(triggered_sensor, 0, sizeof(triggered_sensor));
    memset(triggered_number, 0, sizeof(triggered_number));

    sensor_list_print();
}

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

    // Initialize our sensor thing.
    sensors_init();

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
                        courier = 0;
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
                dassert(false, "Invalid SensorServer Request");
        }
    }

    Exit();
}
