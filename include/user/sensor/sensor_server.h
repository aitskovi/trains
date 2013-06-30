/*
 * sensor_server.h
 *
 *  Created on: Jun 22, 2013
 *      Author: aianus
 */

#ifndef SENSOR_SERVER_H_
#define SENSOR_SERVER_H_

#define SENSOR_LIST_HEIGHT 3
#define SENSOR_DATA_SIZE 10
#define NUM_READINGS 5
#define NUM_SENSORS 5

#include <task.h>

#define MAX_SUBSCRIBERS (MAX_TASKS - 1 / 32) + 1

enum SENSOR_SERVER_MESSAGE_TYPE {
    SENSOR_EVENT_REQUEST,
    SENSOR_EVENT_RESPONSE,
    SENSOR_COURIER_REQUEST,
    SENSOR_COURIER_RESPONSE,
    SENSOR_SUBSCRIBE_REQUEST,
    SENSOR_SUBSCRIBE_RESPONSE,
};

typedef struct SensorServerMessage {
    unsigned int type;

    /*
     * All sensor Data.
     */
    char data[SENSOR_DATA_SIZE];

    /**
     * Single Sensor Data.
     */
    char sensor;
    int number;
    int subscribers[MAX_SUBSCRIBERS];

} SensorServerMessage;

void sensor_server();

#endif /* SENSOR_SERVER_H_ */
