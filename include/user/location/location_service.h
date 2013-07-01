#ifndef _LOCATION_SERVICE_H_
#define _LOCATION_SERVICE_H_

#include <circular_queue.h>
#include <task.h>

#define MAX_PENDING_SENSORS 4
#define MAX_TRAINS 8
#define MAX_SUBSCRIBERS (MAX_TASKS - 1 / 32) + 1

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
int locationservice_sensor_event(struct LocationService *service, char name, int number);
int locationservice_add_train(struct LocationService *service, int train);


#endif
