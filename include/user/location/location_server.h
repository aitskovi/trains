#ifndef _LOCATION_SERVER_H_
#define _LOCATION_SERVER_H_

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
} LocationServerMessage;

void LocationServer();

#endif
