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

enum SENSOR_SERVER_MESSAGE_TYPE {
    SENSOR_EVENT_REQUEST,
    SENSOR_EVENT_RESPONSE,
    SENSOR_DATA_SUBSCRIBE_REQUEST,
    SENSOR_DATA_SUBSCRIBE_RESPONSE,
};

typedef struct SensorServerMessage {
    unsigned int type;
    char data[SENSOR_DATA_SIZE];
} SensorServerMessage;

void sensor_server();

#endif /* SENSOR_SERVER_H_ */
