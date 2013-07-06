#include <train_widget.h>

#include <dassert.h>
#include <log.h>
#include <sprintf.h>
#include <write_server.h>
#include <location_service.h>
#include <track_node.h>
#include <nameserver.h>
#include <encoding.h>
#include <syscall.h>

#define TRAIN_TABLE_HEIGHT 9
#define TRAIN_COLUMN_WIDTH 10

/**
 * Update a specific train.
 */
static int train_display_update(int index, int number, struct track_edge *edge, int distance) {
    char command[128];
    char *pos = &command[0];

    // Draw Train Number.
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_TABLE_HEIGHT + 1, index * TRAIN_COLUMN_WIDTH + 1);
    char num[10];
    sprintf(num, "Train %d", number);
    pos += sputw(pos, TRAIN_COLUMN_WIDTH, ' ', num);

    // Draw Train Position
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_TABLE_HEIGHT + 2, index * TRAIN_COLUMN_WIDTH + 1);
    char position[TRAIN_COLUMN_WIDTH];
    if (!edge) {
        sprintf(position, "N/A");
    } else {
        sprintf(position, "%s %dmm", edge->src->name, distance / 1000);
    }
    pos += sputw(pos, TRAIN_COLUMN_WIDTH, ' ', position);
    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);

    return 0;
}

static int train_display_init() {
    char command[128];
    char *pos = &command[0];
    pos += sprintf(pos, "\0337\033[%u;%uH", TRAIN_TABLE_HEIGHT, 1);
    pos += sprintf(pos, "Trains:");
    pos += sprintf(pos, "\0338");
    Write(COM2, command, pos - command);
    return 0;
}

void train_widget() {

    // Find the location server.
    int location_server_tid = -2;
    do {
        location_server_tid = WhoIs("LocationServer");
        dlog("Location Server Tid %d\n", sensor_server_tid);
    } while (location_server_tid < 0);

    // Subscribe.
    location_server_subscribe(location_server_tid);

    // Initial Display.
    train_display_init();

    // Deal with out Subscription.
    int tid;
    struct Message msg, rply;
    int number_to_train[MAX_TRAINS];
    int num_trains = 0;
    for(;;) {
        // Recieve a Sensor Message.
        Receive(&tid, (char *) &msg, sizeof(msg));
        cuassert(msg.type == LOCATION_SERVER_MESSAGE, "Invalid Message");
        cuassert(msg.ls_msg.type == LOCATION_COURIER_REQUEST, "Invalid Location Widget Request");

        // Ack Location Message.
        rply.type = LOCATION_SERVER_MESSAGE;
        rply.ss_msg.type = LOCATION_COURIER_RESPONSE;
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
        }

        // Update Train List.
        train_display_update(index, msg.ls_msg.data.id,
                                    msg.ls_msg.data.edge,
                                    msg.ls_msg.data.distance);
    }

    Exit();
}
