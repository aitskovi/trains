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
int Unsubscribe(char *name, enum PUBSUB_PRIORITY priority);
int Publish(tid_t tid, struct Message *msg);

enum PUBSUB_MESSAGE_TYPE {
    SUBSCRIBE,
    UNSUBSCRIBE,
    COURIER,
    CONFIGURE,
};

typedef struct PubSubMessage {
    int type;
    union {
        int priority;
        char *name;
    };
} PubSubMessage;

tid_t CreateStream(char *name);

#endif
