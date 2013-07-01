#ifndef _LOCATION_SERVICE_H_
#define _LOCATION_SERVICE_H_

#include <circular_queue.h>
#include <task.h>

#define MAX_PENDING_SENSORS 4
#define MAX_TRAINS 8

struct TrainLocation {
    int number;
    struct track_node *landmark;
    int distance;
    struct track_node *sensors[MAX_PENDING_SENSORS];
};

struct LocationService {
    struct TrainLocation trains[MAX_TRAINS];
    int num_trains;

    struct circular_queue events;
    int subscribers[MAX_SUBSCRIBERS];
};

void locationservice_initialize(struct LocationService *service);

/**
 * Notify the locationservice of a sensor hit.
 */
int locationservice_sensor_event(struct LocationService *service, char name, int number);

/**
 * Add a train for LocationService to track.
 */
int locationservice_add_train(struct LocationService *service, int train);

/**
 * Get an event from the service.
 */
int locationservice_pop(struct LocationService *service, int *train, struct track_node** landmark, int *distance, int *subscribers);

/**
 * Subscribe to LocationService events.
 */
int locationservice_subscribe(struct LocationService *service, int tid);

/**
 * Unsubscribe to LocationService events.
 */
int locationservice_unsubscribe(struct LocationService *service, int tid);


#endif
