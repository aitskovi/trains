#include <calibration.h>

#include <track_node.h>
#include <log.h>
#include <nameserver.h>
#include <location_server.h>
#include <encoding.h>
#include <syscall.h>
#include <dassert.h>
#include <clock_server.h>
#include <pubsub.h>
#include <common.h>
#include <memory.h>

#define MAX_TRAINS 8
#define STOPPING_DISTANCE 11

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

    if (train == 49) return 5683;
    else if (train == 50) return 5213;
    else if (train == 47) return 5311;
    else return 5300;
}

int stopping_distance(int train, int v) {
    if (v == 0) return 0;
    /*
    if (v == velocity(train, 11, 0)) {
        if (train == 49) return 74000;
        else if (train == 50) return 62000;
        else if (train == 47) return 72000;
        else return 70000;
    } else {
    */
    //else return 0.0100 * velocity * velocity + 94.5345 * velocity;
    //else return (0.0100 * velocity * velocity + 74.6926 * velocity);
    else return max(0, 132 * v - 24948);
}

int deceleration(int train, int start, int end, int tick) {
    int d = stopping_distance(train, end) - stopping_distance(train, start);
    int v = end - start;
    return v * v / (2 * d);
}

int acceleration(int train, int start, int end, int tick) {
    if (start > end) return deceleration(train, start, end, tick);
    if (tick < 35) return 0;
    tick = tick - 35;

    double dv = -0.0004 * tick * tick + 0.1756 * tick;
    return max(0, (int)dv + 1);
    /*
    if (tick < 100) return 0;

    int d = stopping_distance(train, end) - stopping_distance(train, start);
    int v = end - start;
    return v * v / (2 * d);
    */
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
    Message msg;
    msg.type = CALIBRATION_MESSAGE;
    msg.cs_msg.type = CALIBRATION_INFO;
    msg.cs_msg.train = train;
    msg.cs_msg.velocity = velocity;
    msg.cs_msg.error = error;
    Publish(tid, &msg);
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

    Subscribe("LocationServerStream", PUBSUB_MEDIUM);

    tid_t stream = CreateStream("CalibrationServerStream");

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
            publish_calibration(stream, data->id, calibration->average_speed, calibration->error);
        }
    }

    Exit();
}
