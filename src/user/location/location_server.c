#include <location_server.h>

#include <dassert.h>
#include <encoding.h>
#include <log.h>
#include <location_service.h>
#include <nameserver.h>
#include <syscall.h>
#include <sensor_server.h>
#include <task.h>
#include <location_courier.h>
#include <distance_notifier.h>

static tid_t server_tid = -1;

int AddTrain(int number) {
    if (server_tid < 0) {
        return -1;
    }
    struct Message msg, reply;
    msg.type = LOCATION_SERVER_MESSAGE;
    msg.ls_msg.type = LOCATION_TRAIN_REQUEST;
    msg.ls_msg.data.id = number;
    Send(server_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    dassert(reply.type == LOCATION_TRAIN_RESPONSE, "Invalid response from switch server");
    return 0;
}

/**
 * Publish a location through the courier.
 */
int location_publish(struct LocationService *service, int tid) {
    struct Message rply;
    rply.type = LOCATION_SERVER_MESSAGE;
    rply.ls_msg.type = LOCATION_COURIER_RESPONSE;

    int result = locationservice_pop_event(service, &rply.ls_msg.data, rply.ls_msg.subscribers);
    if (result == -1) return -1;

    Reply(tid, (char *) &rply, sizeof(rply));

    return 0;
}

void LocationServer() {
    server_tid = MyTid();
    tid_t courier = -1;

    struct LocationService service;
    locationservice_initialize(&service);

    RegisterAs("LocationServer");

    // Create Courier
    Create(HIGH, location_courier);

    // Create a Distance Notifier.
    Create(HIGH, distance_notifier);

    // Subscribe to sensor server.
    Subscribe("SensorServerStream", PUBSUB_HIGH);

    // Start Serving Requests.
    int tid;
    struct Message msg, rply;
    for(;;) {
        Receive(&tid, (char *) &msg, sizeof(msg));

        switch(msg.type) {
            case SENSOR_SERVER_MESSAGE: {
                rply.type = SENSOR_SERVER_MESSAGE;
                SensorServerMessage *ss_msg = &(msg.ss_msg);
                switch(ss_msg->type) {
                    case SENSOR_COURIER_REQUEST:
                        rply.ss_msg.type = SENSOR_COURIER_RESPONSE;
                        Reply(tid, (char *) &rply, sizeof(rply));

                        locationservice_sensor_event(&service, ss_msg->sensor, ss_msg->number);

                        if (courier >= 0) {
                            if (location_publish(&service, courier) != -1) {
                                courier = -1;
                            }
                        }
                        break;
                    default:
                        ulog("\nWarning: Invalid Message Received\n");
                        break;
                }
            }
                break;
            case LOCATION_SERVER_MESSAGE: {
                rply.type = LOCATION_SERVER_MESSAGE;

                LocationServerMessage *ls_msg = &(msg.ls_msg);
                switch(ls_msg->type) {
                    case LOCATION_SUBSCRIBE_REQUEST:
                        rply.ls_msg.type = LOCATION_SUBSCRIBE_RESPONSE;
                        Reply(tid, (char *) &rply, sizeof(rply));

                        locationservice_subscribe(&service, tid);
                        break;
                    case LOCATION_TRAIN_REQUEST:
                        rply.ls_msg.type = LOCATION_TRAIN_RESPONSE;
                        Reply(tid, (char *) &rply, sizeof(rply));

                        locationservice_add_train(&service, ls_msg->data.id);
                        break;
                    case LOCATION_COURIER_REQUEST: {
                        int result = location_publish(&service, tid);
                        if (result == -1) {
                            courier = tid;
                        }
                    }
                        break;
                    case LOCATION_TICK_REQUEST:
                        rply.ls_msg.type = LOCATION_TICK_RESPONSE;
                        Reply(tid, (char *) &rply, sizeof(rply));

                        locationservice_distance_event(&service);
                        break;
                    default:
                        ulog("\nWARNING: Invalid Message Recieved\n");
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

                        // Update our train.
                        locationservice_speed_event(&service, tr_msg->train, tr_msg->speed);
                        
                        break;
                    default:
                        cuassert(0, "Invalid Train Message\n");
                }
                break;
            }
            default:
                ulog("\nWARNING: Invalid Message Received\n");
        }

        if (courier >= 0) {
            if (location_publish(&service, courier) != -1) {
                courier = -1;
            }
        }
    }

    Exit();
}

void location_server_subscribe(int server) {
    struct Message msg, rply;
    msg.type = LOCATION_SERVER_MESSAGE;
    msg.ls_msg.type = LOCATION_SUBSCRIBE_REQUEST;
    Send(server, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
    cuassert(rply.type == LOCATION_SERVER_MESSAGE, "Invalid Response from LocationServer");
    cuassert(rply.ls_msg.type == LOCATION_SUBSCRIBE_RESPONSE, "Invalid Response from Location Server");
}
