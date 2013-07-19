#ifndef _LOCATION_SERVER_H_
#define _LOCATION_SERVER_H_

#include <task.h>

enum LOCATION_SERVER_MESSAGE_TYPE {
    LOCATION_COURIER_REQUEST,       // Courier information.
    LOCATION_COURIER_RESPONSE,
    LOCATION_SUBSCRIBE_REQUEST,     // Subscribe to train location changes.
    LOCATION_SUBSCRIBE_RESPONSE,
    LOCATION_TRAIN_REQUEST,         // Add a train.
    LOCATION_TRAIN_RESPONSE,
    LOCATION_TICK_REQUEST,       // 1 tick timeout.
    LOCATION_TICK_RESPONSE,
};

enum TRAIN_ORIENTATION {
    TRAIN_UNKNOWN = 0,
    TRAIN_FORWARD,
    TRAIN_BACKWARD,
};

struct track_node;

typedef struct TrainData {
    int id;
    int speed;
    int velocity;
    struct track_edge *edge;
    int distance;
    int stopping_distance;
    int error;
    enum TRAIN_ORIENTATION orientation;
} TrainData;

typedef struct LocationServerMessage {
    int type;

    TrainData data;
} LocationServerMessage;

void LocationServer();

int AddTrain(int number);

#endif
