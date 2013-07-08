#include <calibration.h>

#include <track_node.h>
#include <log.h>
#include <nameserver.h>
#include <location_server.h>
#include <encoding.h>
#include <syscall.h>
#include <dassert.h>
#include <clock_server.h>

#define MAX_TRAINS 8

typedef struct TrainCalibration {
    int id;
    int average_speed;
    int error;
    int sensor_distance;
    int time;

    TrainData data;
} TrainCalibration;

int velocity(int train, int speed, track_edge *edge) {
    // TODO: Use the velocity tables.
    if (speed == 0) return 0;
    else return 5300;
}

int stopping_distance(int train, int velocity) {
    if (velocity == 0) return 0;
    else return (0.0100 * velocity * velocity + 74.6926 * velocity);
}

int acceleration(int train, int start, int end, int tick) {
    int d = stopping_distance(train, end) - stopping_distance(train, start);
    int v = end - start;
    ulog("D = %d, V = %d", d, v);
    return v * v / (2 * d);
}

int calibration_error(int train) {
    return 10000;
}

struct location_event {
    int train;
    track_edge *edge;
    int sensor_distance;
    int edge_distance;
    int time;
};

static void publish_calibration(int tid, int train, int velocity, int error) {
    // Notify the distance server of the change.
    Message msg, reply;
    msg.type = CALIBRATION_MESSAGE;
    msg.cs_msg.type = CALIBRATION_INFO;
    msg.cs_msg.train = train;
    msg.cs_msg.velocity = velocity;
    msg.cs_msg.error = error;
    Send(tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    cuassert(CALIBRATION_MESSAGE == reply.type, "Calibration task received invalid msg");
}

int calibration_update(TrainCalibration *calibration, TrainData *data) {
    if (!data->edge) return 0;

    TrainData *old_data = &calibration->data;
    TrainData *new_data = data;

    if (!old_data->edge || old_data->edge == new_data->edge) {
        calibration->data = *new_data;
        return 0;
    }

    // We've passed an edge, add its distance.
    calibration->sensor_distance += old_data->edge->dist;

    // If we're switching to a sensor, do some calibration.
    if (new_data->edge->src->type == NODE_SENSOR) {
        int time = Time();
        int elapsed_time = time - calibration->time;
        int elapsed_sensor_distance = calibration->sensor_distance;
        int elapsed_edge_distance = old_data->distance + old_data->velocity - old_data->edge->dist;
        int elapsed_speed_um_tick = elapsed_sensor_distance / elapsed_time;

        calibration->sensor_distance = 0;
        calibration->data = *new_data;
        calibration->average_speed = elapsed_speed_um_tick;
        calibration->error = elapsed_edge_distance;
        calibration->time = time;

        return 1;
    } else {
        calibration->data = *new_data;
        return 0;
    }
}

void calibration_server() {
    RegisterAs("CalibrationServer");

    tid_t location_server_tid = WhoIs("LocationServer");
    location_server_subscribe(location_server_tid);
    tid_t train_widget_tid = WhoIs("TrainWidget");

    // Deal with our Subscription.
    int tid;
    struct Message msg, rply;

    int number_to_train[MAX_TRAINS];
    TrainCalibration calibrations[MAX_TRAINS];

    int num_trains = 0;
    for(;;) {
        // Recieve a location Message.
        Receive(&tid, (char *) &msg, sizeof(msg));
        cuassert(msg.type == LOCATION_SERVER_MESSAGE, "Invalid Message");
        cuassert(msg.ls_msg.type == LOCATION_COURIER_REQUEST, "Invalid Location Widget Request");

        // Ack Location Message.
        rply.type = LOCATION_SERVER_MESSAGE;
        rply.ls_msg.type = LOCATION_COURIER_RESPONSE;
        Reply(tid, (char *) &rply, sizeof(rply));

        TrainData *data = &msg.ls_msg.data;
        // Find the train index.
        int index;
        for (index = 0; index < num_trains; ++index) {
            if (number_to_train[index] == data->id) break;
        }

        // We couldn't find index, add it instead.
        if (num_trains == index) {
            number_to_train[index] = data->id;
            num_trains++;

            memset(&calibrations[index], 0, sizeof(TrainCalibration));
        }

        TrainCalibration *calibration = &calibrations[index];
        if (calibration_update(calibration, data)) {
            publish_calibration(train_widget_tid, data->id, calibration->average_speed, calibration->error);
        }
    }

    Exit();
}
