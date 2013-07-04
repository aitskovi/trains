#ifndef _LOCATION_SERVICE_H_
#define _LOCATION_SERVICE_H_

#include <circular_queue.h>
#include <task.h>

#define MAX_PENDING_SENSORS 4
#define MAX_TRAINS 8

typedef struct TrainLocation {
    int number;
    struct track_edge *edge;
    int distance;
    struct track_node *next_sensor;
} TrainLocation;

typedef struct LocationService {
    struct TrainLocation trains[MAX_TRAINS];
    int num_trains;

    struct circular_queue events;
    int subscribers[MAX_SUBSCRIBERS];
} LocationService;

void locationservice_initialize(struct LocationService *service);

/**
 * Notify the locationservice of a sensor hit.
 */
int locationservice_sensor_event(struct LocationService *service, char name, int number);

/**
 * Notify the locationservice of 1cm movement.
 */
int locationservice_distance_event(struct LocationService *service, int train);

/**
 * Add a train for LocationService to track.
 */
int locationservice_add_train(struct LocationService *service, int train);

/**
 * Notify our service of the reverse event.
 */
int locationservice_reverse_event(struct LocationService *service, int train);

/**
 * Get an event from the service.
 */
int locationservice_pop(struct LocationService *service, int *train, struct track_node** landmark, struct track_edge** edge, int *distance, int *subscribers);

/**
 * Subscribe to LocationService events.
 */
int locationservice_subscribe(struct LocationService *service, int tid);

/**
 * Unsubscribe to LocationService events.
 */
int locationservice_unsubscribe(struct LocationService *service, int tid);


#endif
