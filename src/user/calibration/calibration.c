#include <calibration.h>

#include <track_node.h>
#include <log.h>
#include <nameserver.h>
#include <location_server.h>
#include <encoding.h>
#include <syscall.h>
#include <dassert.h>

#define MAX_TRAINS 8

struct location_event {
    track_edge *edge;
    int distance;
    int time;
};

void calibration_update(struct location_event *event, track_edge *edge, int distance) {
    if (event->edge == edge) return;
    if (!edge || !edge->src) return;

    event->distance += event->edge->dist;
    event->edge = edge;

    if (edge->src->type == NODE_SENSOR) {
        int time = Time();

        if (event->edge) {
            int elapsed_time = time - event->time;
            ulog("Time: %d ticks", elapsed_time);
            ulog("Distance: %d", event->distance * 1000);
            ulog("Speed: %d um/tick", event->distance * 1000 / elapsed_time);
        }

        event->distance = 0;
        event->time = Time();
    }
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
        }

        calibration_update(&location_events[index], msg.ls_msg.edge, msg.ls_msg.distance);
    }

    Exit();
}
