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
#define NUM_SPEEDS 15
#define MAX_TRAIN_IDS 80

static short velocities[MAX_TRAIN_IDS][NUM_SPEEDS];

typedef struct TrainCalibration {
    int id;
    int average_speed;
    int error;
    int sensor_distance;
    int time;

    TrainData data;
} TrainCalibration;

void initialize_calibration() {
    memset(velocities, 0, sizeof(velocities));

    velocities[45][0] = 0;
    velocities[45][1] = 674;
    velocities[45][2] = 674;
    velocities[45][3] = 1236;
    velocities[45][4] = 1684;
    velocities[45][5] = 2317;
    velocities[45][6] = 2809;
    velocities[45][7] = 3456;
    velocities[45][8] = 3822;
    velocities[45][9] = 4263;
    velocities[45][10] = 4613;
    velocities[45][11] = 5009;
    velocities[45][12] = 5609;
    velocities[45][13] = 5720;
    velocities[45][14] = 5795;

    velocities[46][0] = 0;
    velocities[46][1] = 694;
    velocities[46][2] = 694;
    velocities[46][3] = 1282;
    velocities[46][4] = 1813;
    velocities[46][5] = 2392;
    velocities[46][6] = 2933;
    velocities[46][7] = 3505;
    velocities[46][8] = 3864;
    velocities[46][9] = 4371;
    velocities[46][10] = 4772;
    velocities[46][11] = 5331;
    velocities[46][12] = 5467;
    velocities[46][13] = 5534;
    velocities[46][14] = 5575;

    velocities[47][0] = 0;
    velocities[47][1] = 694;
    velocities[47][2] = 694;
    velocities[47][3] = 1282;
    velocities[47][4] = 1813;
    velocities[47][5] = 2392;
    velocities[47][6] = 2933;
    velocities[47][7] = 3505;
    velocities[47][8] = 3864;
    velocities[47][9] = 4371;
    velocities[47][10] = 4772;
    velocities[47][11] = 5331;
    velocities[47][12] = 5467;
    velocities[47][13] = 5534;
    velocities[47][14] = 5575;

    velocities[48][0] = 0;
    velocities[48][1] = 637;
    velocities[48][2] = 637;
    velocities[48][3] = 1239;
    velocities[48][4] = 1780;
    velocities[48][5] = 2345;
    velocities[48][6] = 2895;
    velocities[48][7] = 3382;
    velocities[48][8] = 3800;
    velocities[48][9] = 4253;
    velocities[48][10] = 4719;
    velocities[48][11] = 5252;
    velocities[48][12] = 5592;
    velocities[48][13] = 5630;
    velocities[48][14] = 5703;

    velocities[49][0] = 0;
    velocities[49][1] = 737;
    velocities[49][2] = 737;
    velocities[49][3] = 1335;
    velocities[49][4] = 1840;
    velocities[49][5] = 2359;
    velocities[49][6] = 2846;
    velocities[49][7] = 3443;
    velocities[49][8] = 3968;
    velocities[49][9] = 4601;
    velocities[49][10] = 5069;
    velocities[49][11] = 5660;
    velocities[49][12] = 6207;
    velocities[49][13] = 6308;
    velocities[49][14] = 6362;

    velocities[50][0] = 0;
    velocities[50][1] = 660;
    velocities[50][2] = 660;
    velocities[50][3] = 1253;
    velocities[50][4] = 1753;
    velocities[50][5] = 2213;
    velocities[50][6] = 2733;
    velocities[50][7] = 3286;
    velocities[50][8] = 3827;
    velocities[50][9] = 4340;
    velocities[50][10] = 4827;
    velocities[50][11] = 5286;
    velocities[50][12] = 5767;
    velocities[50][13] = 5968;
    velocities[50][14] = 6141;
}


int velocity(int train, int speed, track_edge *edge) {
    cuassert(train >= 45 && train <= 50, "Using uncalibrated train!");

    return velocities[train][speed];
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
    else return max(0, 123 * v - 12584);
    //else return max(0, 132 * v - 24948);
}

int deceleration(int train, int start, int end, int tick) {
    int d = stopping_distance(train, end) - stopping_distance(train, start);
    int v = end - start;
    return v * v / (2 * d);
}

int acceleration(int train, int start, int end, int tick) {
    if (start > end) return deceleration(train, start, end, tick);
    if (tick < 60) return 0;
    tick = tick - 60;

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
    initialize_calibration();

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
