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
#define TRAIN_COLUMN_WIDTH 12

/**
 * Update a specific train.
 */
static int train_position_update(int index, int number, struct track_edge *edge, int distance, int velocity, int stopping_distance) {
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

    // Draw Calibrated Train Velocity
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_TABLE_HEIGHT + 3, index * TRAIN_COLUMN_WIDTH + 1);
    char velocity_buf[TRAIN_COLUMN_WIDTH];
    sprintf(velocity_buf, "%dum/tick", velocity);
    pos += sputw(pos, TRAIN_COLUMN_WIDTH, ' ', velocity_buf);
    pos += sprintf(pos, "\0338");

    // Draw Calibrated Train Stopping Distance
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_TABLE_HEIGHT + 4, index * TRAIN_COLUMN_WIDTH + 1);
    char stop_buf[TRAIN_COLUMN_WIDTH];
    sprintf(stop_buf, "%dum", stopping_distance);
    pos += sputw(pos, TRAIN_COLUMN_WIDTH, ' ', stop_buf);
    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);

    return 0;
}

static int train_calibration_update(int index, int number, int velocity, int error) {
    char command[128];
    char *pos = &command[0];

    // Draw Train Speed
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_TABLE_HEIGHT + 6, index * TRAIN_COLUMN_WIDTH + 1);
    char velocity_buf[TRAIN_COLUMN_WIDTH];
    sprintf(velocity_buf, "%dum/tick", velocity);
    pos += sputw(pos, TRAIN_COLUMN_WIDTH, ' ', velocity_buf);

    // Draw Train Error.
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_TABLE_HEIGHT + 7, index * TRAIN_COLUMN_WIDTH + 1);
    char error_buf[TRAIN_COLUMN_WIDTH];
    sprintf(error_buf, "%dmm", error / 1000);
    pos += sputw(pos, TRAIN_COLUMN_WIDTH, ' ', error_buf);

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

    pos += sprintf(pos, "\0337\033[%u;%uH", TRAIN_TABLE_HEIGHT + 5, 1);
    pos += sprintf(pos, "Calibration:");
    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
    return 0;
}

static int get_train_index(int *number_to_train, int *num_trains, int id) {
    // Find the train index.
    int index;
    for (index = 0; index < *num_trains; ++index) {
        if (number_to_train[index] == id) break;
    }

    // We couldn't find index, add it instead.
    if (*num_trains == index) {
        number_to_train[index] = id;
        *num_trains = *num_trains + 1;
    }

    return index;
}

void train_widget() {
    RegisterAs("TrainWidget");

    // Find the location server and subscribe.
    tid_t location_server_tid = WhoIs("LocationServer");
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

        switch(msg.type) {
            case LOCATION_SERVER_MESSAGE: {
                cuassert(msg.ls_msg.type == LOCATION_COURIER_REQUEST, "Invalid Location Widget Request");

                // Ack Location Message.
                rply.type = LOCATION_SERVER_MESSAGE;
                rply.ls_msg.type = LOCATION_COURIER_RESPONSE;
                Reply(tid, (char *) &rply, sizeof(rply));

                TrainData* data = &msg.ls_msg.data;
                int index = get_train_index(number_to_train, &num_trains, data->id);
                train_position_update(index, data->id, data->edge, data->distance, data->velocity, data->stopping_distance);

                break;
            }
            case CALIBRATION_MESSAGE: {
                cuassert(msg.cs_msg.type == CALIBRATION_INFO, "Invalid Location Widget Request");

                // Ack Calibration Message.
                rply.type = CALIBRATION_MESSAGE;
                rply.cs_msg.type = CALIBRATION_INFO;
                Reply(tid, (char *) &rply, sizeof(rply));

                int index = get_train_index(number_to_train, &num_trains, msg.cs_msg.train);
                train_calibration_update(index, msg.cs_msg.train, msg.cs_msg.velocity, msg.cs_msg.error);
                break;
            }
            default:
                cuassert(0, "Invalid Train Widget Message");
        }
    }

    Exit();
}
