#ifndef _DISTANCE_SERVER_H_
#define _DISTANCE_SERVER_H_

#include <task.h>

enum DISTANCE_MESSAGE_TYPE {
    DISTANCE_COURIER_REQUEST,
    DISTANCE_COURIER_RESPONSE,
    DISTANCE_SUBSCRIBE_REQUEST,
    DISTANCE_SUBSCRIBE_RESPONSE,
    DISTANCE_TIMEOUT_REQUEST,
    DISTANCE_TIMEOUT_RESPONSE,
};

typedef struct DistanceServerMessage {
    int type;
    int train;
    int distance;
    int subscribers[MAX_SUBSCRIBERS];
} DistanceServerMessage;

void distance_server();
void distance_server_subscribe(int server);

#endif
