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
#include <distance_server.h>

static tid_t server_tid = -1;

int AddTrain(int number) {
    if (server_tid < 0) {
        return -1;
    }
    struct Message msg, reply;
    msg.type = LOCATION_SERVER_MESSAGE;
    msg.ls_msg.type = LOCATION_TRAIN_REQUEST;
    msg.ls_msg.train = number;
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

    int result = locationservice_pop(service, &(rply.ls_msg.train), &(rply.ls_msg.landmark), &(rply.ls_msg.edge), &(rply.ls_msg.distance), rply.ls_msg.subscribers);
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
    tid_t distance_server_tid = Create(HIGH, distance_server);
    distance_server_subscribe(distance_server_tid);

    // Find the sensor server and subscribe to it.
    int sensor_server_tid = -2;
    do {
        sensor_server_tid = WhoIs("SensorServer");
        dlog("Sensor Server Tid %d\n", sensor_server_tid);
    } while (sensor_server_tid < 0);
    sensor_server_subscribe(sensor_server_tid);

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

                        locationservice_add_train(&service, ls_msg->train);

                        if (courier >= 0) {
                            if (location_publish(&service, courier) != -1) {
                                courier = -1;
                            }
                        }
                        break;
                    case LOCATION_COURIER_REQUEST: {
                        int result = location_publish(&service, tid);
                        if (result == -1) {
                            courier = tid;
                        }
                    }
                        break;
                    default:
                        ulog("\nWARNING: Invalid Message Recieved\n");
                        break;
                }
            }
                break;
            case DISTANCE_SERVER_MESSAGE: {
                rply.type = DISTANCE_SERVER_MESSAGE;

                DistanceServerMessage *ds_msg = &msg.ds_msg;
                switch(ds_msg->type) {
                    case DISTANCE_COURIER_REQUEST:
                        rply.ds_msg.type = DISTANCE_COURIER_RESPONSE;
                        Reply(tid, (char *) &rply, sizeof(rply));

                        locationservice_distance_event(&service, ds_msg->train);
                        if (courier >= 0) {
                            if (location_publish(&service, courier) != -1) {
                                courier = -1;
                            }
                        }
                        break;
                    default:
                        cuassert(0, "Invalid Distance Message for Location Server\n");
                }
            }
                break;
            case TRAIN_MESSAGE: {
                TrainMessage *tr_msg = &msg.tr_msg;
                switch(tr_msg->type) {
                    case COMMAND_REVERSE:
                        // Unblock train task.
                        rply.type = TRAIN_MESSAGE;
                        Reply(tid, (char *)&rply, sizeof(rply));

                        // Update our train.
                        locationservice_reverse_event(&service, tr_msg->train);
                        if (courier >= 0) {
                            if (location_publish(&service, courier) != -1) {
                                courier = -1;
                            }
                        }
                        break;
                    default:
                        cuassert(0, "Invalid Train Message\n");
                }
                break;
            }
            default:
                ulog("\nWARNING: Invalid Message Received\n");
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
