#ifndef _SENSOR_SERVICE_
#define _SENSOR_SERVICE_

#include <sensor_server.h>
#include <circular_queue.h>

struct circular_queue;

struct SensorService {
    int subscribers[MAX_SUBSCRIBERS];
    struct circular_queue sensor_data[NUM_SENSORS];
};

char int_to_sensor(int i);
int sensor_to_int(char c);

/**
 * Add a sensor hit to the service.
 */
int sensorservice_push(struct SensorService *service, char sensor, int number);

/**
 * Grab a sensor hit from the service.
 */
int sensorservice_pop(struct SensorService *service, char *sensor, int *number, int *subscribers);

/**
 * Subscribe to sensor hits.
 */
int sensorservice_subscribe(struct SensorService *service, int tid);

/**
 * Unsubscribe from sensor hits.
 */
int sensorservice_unsubscribe(struct SensorService *service, int tid);

#endif

