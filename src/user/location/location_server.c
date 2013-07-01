#include <location_server.h>

#include <dassert.h>
#include <encoding.h>
#include <log.h>
#include <location_service.h>
#include <nameserver.h>
#include <syscall.h>
#include <sensor_server.h>
#include <task.h>

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

void LocationServer() {
    server_tid = MyTid();
    tid_t courier = -1;

    struct LocationService service;
    locationservice_initialize(&service);

    RegisterAs("LocationServer");

    // TODO: Add Courier
    // Create(HIGH, location_courier);

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
                        break;
                    default:
                        ulog("Warning: Invalid Message Received\n");
                        break;
                }
            }
                break;
            case LOCATION_SERVER_MESSAGE: {
                rply.type = LOCATION_SERVER_MESSAGE;

                LocationServerMessage *ls_msg = &(msg.ls_msg);
                switch(ls_msg->type) {
                    case LOCATION_SUBSCRIBE_REQUEST:
                        // TODO: Add Subscribe API
                        // locationservice_subscribe(...);
                        break;
                    case LOCATION_TRAIN_REQUEST:
                        rply.ls_msg.type = LOCATION_TRAIN_RESPONSE;
                        Reply(tid, (char *) &rply, sizeof(rply));

                        locationservice_add_train(&service, ls_msg->train);
                        break;
                    case LOCATION_COURIER_REQUEST:
                        // TODO: Add Courier API
                        //locationservice_pop(...);
                        break;
                }
            }
                break;
            default:
                ulog("WARNING: Invalid Message Received\n");
        }
    }

    Exit();
}
