#include <pubsub.h>

#include <courier.h>
#include <dassert.h>
#include <encoding.h>
#include <memory.h>
#include <memcpy.h>
#include <nameserver.h>
#include <nameservice.h>
#include <string.h>
#include <syscall.h>

#define MAX_MESSAGES 100

#define min(a,b) (a) < (b) ? (a) : (b)

int Subscribe(char *name, enum PUBSUB_PRIORITY priority) {
    Message msg, rply;
    msg.type = PUBSUB_MESSAGE;
    msg.ps_msg.type = SUBSCRIBE;
    msg.ps_msg.priority = priority;

    char stream_buf[MAX_NAME_LENGTH + 6];
    char *pos = stream_buf;
    pos += strcpy(name, pos);
    pos += strcpy("Stream", pos);

    tid_t server = WhoIs(stream_buf);

    Send(server, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
    cuassert(rply.type == PUBSUB_MESSAGE, "Non pubsub message from pubsub server");

    return 0;
}

int Unsubscribe(char *name, enum PUBSUB_PRIORITY priority) {
    Message msg, rply;
    msg.type = PUBSUB_MESSAGE;
    msg.ps_msg.type = UNSUBSCRIBE;
    msg.ps_msg.priority = priority;

    char stream_buf[MAX_NAME_LENGTH + 6];
    char *pos = stream_buf;
    pos += strcpy(name, pos);
    pos += strcpy("Stream", pos);

    tid_t server = WhoIs(stream_buf);

    Send(server, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
    cuassert(rply.type == PUBSUB_MESSAGE, "Non pubsub message from pubsub server");

    return 0;
}

int Publish(tid_t tid, Message *msg) {
    Message rply;

    Send(tid, (char *)msg, sizeof(Message), (char *)&rply, sizeof(rply));
    cuassert(rply.type == PUBSUB_MESSAGE, "Non pubsub message from pubsub server");

    return 0;
}

typedef struct MsgMultiQueue {
    unsigned int read_count[PUBSUB_NUM_PRIORITIES];
    unsigned int write_count;

    Message msgs[MAX_MESSAGES];
} MsgMultiQueue;

typedef struct PubSubService {
    MsgMultiQueue queue;

    int subscribers[PUBSUB_NUM_PRIORITIES][MAX_SUBSCRIBERS];
} PubSubService;

/**************************
 *      MsgMultiQueue     *
 **************************/

static unsigned int multiqueue_min_read_count(MsgMultiQueue *queue) {
    int readmin = queue->read_count[0];
    int i;
    for (i = 0; i < PUBSUB_NUM_PRIORITIES; ++i) {
        readmin = min(readmin, queue->read_count[i]);
    }

    return readmin;
}

static int multiqueue_empty(MsgMultiQueue *queue, enum PUBSUB_PRIORITY priority) {
    return queue->read_count[priority] == queue->write_count;
}

static Message multiqueue_pop(MsgMultiQueue *queue, enum PUBSUB_PRIORITY priority) {
    if (!multiqueue_empty(queue, priority)) {
        return queue->msgs[queue->read_count[priority]++ % MAX_MESSAGES];
    }

    cuassert(0, "Not good");
    Message msg;
    return msg;
}

static int multiqueue_push(MsgMultiQueue *queue, Message *msg) {
    queue->msgs[queue->write_count++ % MAX_MESSAGES] = *msg;

   // Check if we've overrun our queue
   if (queue->write_count - multiqueue_min_read_count(queue) > MAX_MESSAGES) {
       return -1;
   }

   return 0;
}

static void multiqueue_initialize(MsgMultiQueue *queue) {
    memset(queue->msgs, 0, sizeof(queue->msgs));
    memset(queue->read_count, 0, sizeof(queue->read_count));
    queue->write_count = 0;
}

/*
static unsigned int multiqueue_size(MsgMultiQueue *queue, enum PUBSUB_PRIORITY priority) {
    return queue->write_count - queue->read_count[priority];
}
*/

/**************************
 *      PubSubService     *
 **************************/

static int pubsubservice_subscribe(PubSubService *service, int tid, enum PUBSUB_PRIORITY priority) {
    int section = tid / 32;
    int bit = 1 << (tid % 32);

    service->subscribers[priority][section] |= bit;

    return 0;
}

static int pubsubservice_unsubscribe(PubSubService *service, int tid, enum PUBSUB_PRIORITY priority) {
    int section = tid / 32;
    int bit  = 1 << (tid % 32);

    service->subscribers[priority][section] &= ~bit;

    return 0;
}

static void pubsubservice_subscribers(PubSubService *service, int *subscribers, enum PUBSUB_PRIORITY priority) {
    memcpy(subscribers, service->subscribers[priority], sizeof(MAX_SUBSCRIBERS * sizeof(int)));
}

static int pubsubservice_pop(PubSubService *service, Message *msg, enum PUBSUB_PRIORITY priority) {
    if (multiqueue_empty(&service->queue, priority)) {
        return -1;
    } else {
        *msg = multiqueue_pop(&service->queue, priority);
        return 0;
    }
}

static int pubsubservice_push(PubSubService *service, Message *msg) {
    return multiqueue_push(&service->queue, msg);
}

static int pubsubservice_empty(PubSubService *service, enum PUBSUB_PRIORITY priority) {
    return multiqueue_empty(&service->queue, priority);
}

static void pubsubservice_initialize(PubSubService *service) {
    multiqueue_initialize(&service->queue);
    int i;
    for (i = 0; i < PUBSUB_NUM_PRIORITIES; ++i) {
        int j;
        for (j = 0; j < MAX_SUBSCRIBERS; ++j) {
            service->subscribers[i][j] = -1;
        }
    }
}

/**************************
 *       PubSubServer     *
 **************************/

static void pubsub_publish(PubSubService *service, int *couriers) {
    int priority;
    for (priority = 0; priority < PUBSUB_NUM_PRIORITIES; ++priority) {
        if (couriers[priority] != -1 && !pubsubservice_empty(service, priority)) {
            Message rply;
            pubsubservice_pop(service, &rply, priority);
            pubsubservice_subscribers(service, rply.subscribers, priority);
            Reply(couriers[priority], (char *)&rply, sizeof(rply));
            couriers[priority] = -1;
        }
    }
}

static void ConfigureStream() {
    tid_t tid;
    PubSubMessage msg, rply;

    Receive(&tid, (char *)&msg, sizeof(msg));
    cuassert(msg.type == CONFIGURE, "Received invalid message");

    RegisterAs(msg.name);

    Reply(tid, (char *)&rply, sizeof(rply));
}

static void PubSubServer() {
    PubSubService service;
    pubsubservice_initialize(&service);

    ConfigureStream();

    tid_t couriers[PUBSUB_NUM_PRIORITIES];
    int i;
    for (i = 0; i < PUBSUB_NUM_PRIORITIES; ++i) {
        Execute(HIGHEST, courier, i);
    }

    int tid; 
    Message msg, rply;
    for (;;) {
        Receive(&tid, (char *)&msg, sizeof(msg));
        cuassert(msg.type == PUBSUB_MESSAGE, "Invalid Message for PubSub Server");

        switch(msg.ps_msg.type) {
            case SUBSCRIBE:
                rply.type = PUBSUB_MESSAGE;
                Reply(tid, (char *)&rply, sizeof(rply));

                pubsubservice_subscribe(&service, tid, msg.ps_msg.priority);
                break;
            case UNSUBSCRIBE:
                rply.type = PUBSUB_MESSAGE;
                Reply(tid, (char *)&rply, sizeof(rply));

                pubsubservice_unsubscribe(&service, tid, msg.ps_msg.priority);
                break;
            case COURIER:
                couriers[msg.ps_msg.priority] = tid;
                break;
            default:
                // By default we get a publish.
                rply.type = PUBSUB_MESSAGE;
                Reply(tid, (char *)&rply, sizeof(rply));

                pubsubservice_push(&service, &msg);
                break;
        }

        pubsub_publish(&service, couriers);
    }

    Exit();
}

tid_t CreateStream(char *name) {
    tid_t server = Create(HIGHEST, PubSubServer);

    PubSubMessage msg, rply;
    msg.type = CONFIGURE;
    msg.name = name;
    Send(server, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
    cuassert(rply.type == CONFIGURE, "Invalid Configuration Reply");

    return server;
}
