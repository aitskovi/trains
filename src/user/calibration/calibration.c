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

struct location_event {
    track_edge *edge;
    int sensor_distance;
    int edge_distance;
    int time;
};

void calibration_update(struct location_event *event, track_edge *edge, int distance) {
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
        int elapsed_sensor_distance_mm = event->sensor_distance;
        int elapsed_edge_distance_um = event->edge_distance + 5300 - event->edge->dist * 1000;
        int elapsed_edge_distance_mm = elapsed_edge_distance_um / 1000;
        ulog("Time: %d ticks", elapsed_time);
        ulog("Distance: %d mm", event->sensor_distance);
        ulog("Speed: %d um/tick", elapsed_sensor_distance_mm * 1000 / elapsed_time);
        ulog("Error: %d mm", elapsed_edge_distance_mm);

        event->sensor_distance = 0;
        event->time = Time();
    }

    event->edge = edge;
}

void calibration_server() {
    RegisterAs("CalibrationServer");

    tid_t location_server_tid = WhoIs("LocationServer");
    location_server_subscribe(location_server_tid);

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
            if (number_to_train[index] == msg.ls_msg.train) break;
        }

        // We couldn't find index, add it instead.
        if (num_trains == index) {
            number_to_train[index] = msg.ls_msg.train;
            num_trains++;
            location_events[index].edge = 0;
            location_events[index].sensor_distance = 0;
            location_events[index].edge_distance = 0;
            location_events[index].time = 0;
        }

        calibration_update(&location_events[index], msg.ls_msg.edge, msg.ls_msg.distance);
    }

    Exit();
}
