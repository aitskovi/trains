#ifndef _PUBSUB_H_
#define _PUBSUB_H_

#include <task.h>

struct Message;

enum PUBSUB_PRIORITY {
    PUBSUB_LOW,
    PUBSUB_MEDIUM,
    PUBSUB_HIGH,
    PUBSUB_NUM_PRIORITIES,
};

/**
 * PubSub Server Interface.
 */
int Subscribe(char *name, enum PUBSUB_PRIORITY priority);
int Unsubscribe(char *name);
int Publish(tid_t tid, struct Message *msg);

enum PUBSUB_MESSAGE_TYPE {
    SUBSCRIBE,
    UNSUBSCRIBE,
    COURIER,
};

typedef struct PubSubMessage {
    int type;
    int priority;
} PubSubMessage;

void PubSubServer();

#endif
