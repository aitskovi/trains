#ifndef _LOCATION_SERVICE_H_
#define _LOCATION_SERVICE_H_

#include <circular_queue.h>
#include <task.h>

#define MAX_PENDING_SENSORS 4
#define MAX_TRAINS 8
#define MAX_TRAIN_IDS 80

typedef struct TrainAcceleration {
    int start;
    int end;
    int ticks;
} TrainAcceleration;

typedef struct TrainLocation {
    int id;

    struct track_edge *edge;
    int distance;

    int velocity;
    int speed;

    int accelerating;
    TrainAcceleration acceleration;  /* Train's Acceleration */

    struct track_node *next_sensor;
} TrainLocation;

typedef struct LocationService {
    struct TrainLocation trains[MAX_TRAINS];
    int num_trains;

    int train_to_index[MAX_TRAIN_IDS];

    struct circular_queue events;
    int subscribers[MAX_SUBSCRIBERS];
} LocationService;

void locationservice_initialize(struct LocationService *service);

void locationservice_associate(LocationService *service, TrainLocation *train, struct track_edge *edge);

int locationservice_sensor_event(struct LocationService *service, char name, int number);
int locationservice_distance_event(struct LocationService *service);
int locationservice_speed_event(struct LocationService *service, int train_number, int speed);

int locationservice_add_train(struct LocationService *service, int train_number);

void locationservice_add_event(LocationService *service, TrainLocation *train);
int locationservice_pop_event(
        struct LocationService *service, 
        int *train,
        struct track_edge** edge,
        int *distance,
        int *subscribers);


int locationservice_subscribe(struct LocationService *service, int tid);
int locationservice_unsubscribe(struct LocationService *service, int tid);

#endif
