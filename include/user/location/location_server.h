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
};

struct track_node;

typedef struct LocationServerMessage {
    int type;

    int train;
    struct track_node *landmark;
    int distance;

    int subscribers[MAX_SUBSCRIBERS];
} LocationServerMessage;

void LocationServer();

/** Subscribe to the LocationServer */
void location_server_subscribe(int server);

int AddTrain(int number);

#endif
