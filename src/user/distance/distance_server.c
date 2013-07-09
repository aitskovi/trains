#include <distance_server.h>

#include <dassert.h>
#include <location_service.h>
#include <distance_service.h>
#include <distance_notifier.h>
#include <distance_courier.h>
#include <encoding.h>
#include <nameserver.h>
#include <heap_priority_queue.h>
#include <task.h>
#include <syscall.h>

int distance_publish(struct DistanceService *service, tid_t tid) {
    struct Message rply;
    rply.type = DISTANCE_SERVER_MESSAGE;
    rply.ds_msg.type = DISTANCE_COURIER_RESPONSE;

    int result = distanceservice_event(service, &(rply.ds_msg.train), rply.ds_msg.subscribers);
    if (result < 0) return -1;

    Reply(tid, (char *)&rply, sizeof(rply));
    return 0;
}

int distance_notifier_timeout(int timeout, tid_t tid) {
    if (tid < 0) return -1;
    if (timeout < 0) return tid;

    struct Message rply;
    rply.type = DISTANCE_SERVER_MESSAGE;
    rply.ds_msg.type = DISTANCE_TIMEOUT_RESPONSE;
    rply.ds_msg.time = timeout;
    Reply(tid, (char *)&rply, sizeof(rply));

    return -1;
}

void distance_server() {
    tid_t courier = -1;
    tid_t notifier = -1;
    struct DistanceService service;
    PriorityQueueElement buffer[MAX_TRAINS];
    distanceservice_initialize(&service, buffer, MAX_TRAINS);

    RegisterAs("DistanceServer");

    Create(HIGH, distance_notifier);
    Create(HIGH, distance_courier);

    int tid;
    struct Message msg, rply;
    for (;;) {
        Receive(&tid, (char *) &msg, sizeof(msg));

        switch(msg.type) {
            case DISTANCE_SERVER_MESSAGE: {
                DistanceServerMessage *ds_msg = &(msg.ds_msg);
                switch(ds_msg->type) {
                    case DISTANCE_COURIER_REQUEST:
                        if (distance_publish(&service, tid) == -1) {
                            courier = tid;
                        }
                        break;
                    case DISTANCE_SUBSCRIBE_REQUEST:
                        rply.type = DISTANCE_SERVER_MESSAGE;
                        rply.ds_msg.type = DISTANCE_SUBSCRIBE_RESPONSE;
                        Reply(tid, (char *)&rply, sizeof(rply));

                        distanceservice_subscribe(&service, tid);
                        break;
                    case DISTANCE_TIMEOUT_REQUEST:
                        rply.type = DISTANCE_SERVER_MESSAGE;
                        rply.ds_msg.type = DISTANCE_SUBSCRIBE_RESPONSE;
                        Reply(tid, (char *)&rply, sizeof(rply));

                        distanceservice_notify(&service);
                        break;
                    default:
                        ulog("Invalid Distance Server Message\n");
                        break;
                }
            }
                break;
            case TRAIN_MESSAGE: {
                TrainMessage *tr_msg = &msg.tr_msg;
                switch(tr_msg->type) {
                    case COMMAND_SET_SPEED:
                        // Unblock train task.
                        rply.type = TRAIN_MESSAGE;
                        Reply(tid, (char *)&rply, sizeof(rply));

                        // Update our timeouts.
                        distanceservice_update_train(&service, tr_msg->train, tr_msg->speed);
                        break;
                    default:
                        cuassert(0, "Invalid Train Message\n");
                }
                break;
            }
            default:
                ulog("Invalid Distance Server Message\n");
        }

        // Try to publish a message if we can.
        if (courier >= 0) {
            if (distance_publish(&service, courier) != -1) {
                courier = -1;
            }
        }
    }
}

void distance_server_subscribe(int server) {
    struct Message msg, rply;
    msg.type = DISTANCE_SERVER_MESSAGE;
    msg.ls_msg.type = DISTANCE_SUBSCRIBE_REQUEST;
    Send(server, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
    cuassert(rply.type == DISTANCE_SERVER_MESSAGE, "Invalid Response from LocationServer");
    cuassert(rply.ls_msg.type == DISTANCE_SUBSCRIBE_RESPONSE, "Invalid Response from Location Server");
}
