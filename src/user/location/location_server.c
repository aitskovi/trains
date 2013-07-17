#include <location_server.h>

#include <dassert.h>
#include <encoding.h>
#include <log.h>
#include <location_service.h>
#include <nameserver.h>
#include <syscall.h>
#include <sensor_server.h>
#include <task.h>
#include <distance_notifier.h>
#include <pubsub.h>

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

void LocationServer() {
    server_tid = MyTid();

    RegisterAs("LocationServer");

    // Create a Distance Notifier.
    Create(HIGH, distance_notifier);

    // Subscribe to sensor server.
    Subscribe("SensorServerStream", PUBSUB_HIGH);

    // Create a LocationStream.
    tid_t stream = CreateStream("LocationServerStream");

    LocationService service;
    locationservice_initialize(&service, stream);

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
                        ulog("\nWarning: Invalid Message Received\n");
                        break;
                }
            }
                break;
            case LOCATION_SERVER_MESSAGE: {
                rply.type = LOCATION_SERVER_MESSAGE;

                LocationServerMessage *ls_msg = &(msg.ls_msg);
                switch(ls_msg->type) {
                    case LOCATION_TRAIN_REQUEST:
                        rply.ls_msg.type = LOCATION_TRAIN_RESPONSE;
                        Reply(tid, (char *) &rply, sizeof(rply));

                        locationservice_add_train(&service, ls_msg->data.id);
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
            case SHELL_MESSAGE: {
                ShellMessage *sh_msg = &msg.sh_msg;
                switch(sh_msg->type) {
                    case SHELL_ORIENT:
                        rply.type = SHELL_MESSAGE;
                        rply.sh_msg.type = SHELL_SUCCESS_REPLY;
                        Reply(tid, (char *)&rply, sizeof(rply));

                        locationservice_orientation_event(&service, sh_msg->train_no, sh_msg->orientation);
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
