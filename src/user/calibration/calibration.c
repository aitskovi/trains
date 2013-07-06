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

int velocity(int train, int speed, track_edge *edge) {
    // TODO: Use the velocity tables.
    if (speed == 0) return 0;
    else return 5300;
}

int stopping_distance(int train, int velocity) {
    if (velocity == 0) return 0;
    else return 700000;
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

void calibration_update(struct location_event *event, track_edge *edge, int distance, tid_t tid) {
    if (!edge) return;

    if (!event->edge) {
        event->edge = edge;
        event->edge_distance = distance;
        return;
    }

    if (event->edge == edge) {
        event->edge_distance = distance;
        return;
    }

    event->sensor_distance += event->edge->dist;

    if (edge->src->type == NODE_SENSOR) {
        int time = Time();

        int elapsed_time = time - event->time;
        int elapsed_sensor_distance_um = event->sensor_distance;
        int elapsed_edge_distance_um = event->edge_distance + 5300 - event->edge->dist;
        int elapsed_speed_um_tick = elapsed_sensor_distance_um / elapsed_time;
        int elapsed_edge_distance_mm = elapsed_edge_distance_um / 1000;
        /*
        ulog("Time: %d ticks", elapsed_time);
        ulog("Distance: %d mm", event->sensor_distance);
        ulog("Speed: %d um/tick", elapsed_speed_um_tick);
        ulog("Error: %d mm", elapsed_edge_distance_mm);
        */

        event->sensor_distance = 0;
        event->time = Time();

        publish_calibration(tid, event->train, elapsed_speed_um_tick, elapsed_edge_distance_um);
    }

    event->edge = edge;
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
    struct location_event location_events[MAX_TRAINS];
    int errors[MAX_TRAINS];

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

        // Find the train index.
        int index;
        for (index = 0; index < num_trains; ++index) {
            if (number_to_train[index] == msg.ls_msg.data.id) break;
        }

        // We couldn't find index, add it instead.
        if (num_trains == index) {
            number_to_train[index] = msg.ls_msg.data.id;
            num_trains++;
            location_events[index].train = msg.ls_msg.data.id;
            location_events[index].edge = 0;
            location_events[index].sensor_distance = 0;
            location_events[index].edge_distance = 0;
            location_events[index].time = 0;
        }

        calibration_update(&location_events[index], msg.ls_msg.data.edge, msg.ls_msg.data.distance, train_widget_tid);
    }

    Exit();
}
