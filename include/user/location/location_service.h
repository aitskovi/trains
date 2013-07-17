#ifndef _LOCATION_SERVICE_H_
#define _LOCATION_SERVICE_H_

#include <circular_queue.h>
#include <location_server.h>
#include <task.h>

#define MAX_PENDING_SENSORS 4
#define MAX_TRAINS 8
#define MAX_TRAIN_IDS 80

struct TrainData;

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

    struct track_node *next_sensors[MAX_PENDING_SENSORS];
    int num_pending_sensors;

    enum TRAIN_ORIENTATION orientation;
} TrainLocation;

typedef struct LocationService {
    struct TrainLocation trains[MAX_TRAINS];
    int num_trains;

    int train_to_index[MAX_TRAIN_IDS];

    tid_t stream;
} LocationService;

void locationservice_initialize(struct LocationService *service, tid_t publisher);

void locationservice_associate(LocationService *service, TrainLocation *train, struct track_edge *edge);

int locationservice_sensor_event(LocationService *service, char name, int number);
int locationservice_distance_event(LocationService *service);
int locationservice_speed_event(LocationService *service, int train_number, int speed);
int locationservice_orientation_event(LocationService *service, int train_number, enum TRAIN_ORIENTATION orientation);

int locationservice_add_train(LocationService *service, int train_number);
#endif
