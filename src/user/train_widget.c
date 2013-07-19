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
#include <pubsub.h>
#include <string.h>
#include <memory.h>

#define TRAIN_TABLE_HEIGHT 9
#define TRAIN_COLUMN_WIDTH 23
#define TRAIN_COLUMN_WRITABLE_WIDTH TRAIN_COLUMN_WIDTH - 1
#define MAX_RESERVED_NODES 10

#define TRAIN_STRING "Train:"
#define LANDMARK_STRING "Landmark:"
#define DISTANCE_STRING "Distance:"
#define ESTIMATED_SPEED_STRING "Est. Speed:"
#define STOPPING_DISTANCE_STRING "Stop Dist.:"
#define MEASURED_VELOCITY_STRING "Mea. Speed:"
#define ERROR_STRING "Error:"
#define ORIENTATION_STRING "Orientation:"

enum TRAIN_WIDGET_HEIGHTS {
    TRAIN_DISPLAY_HEIGHT = TRAIN_TABLE_HEIGHT,
    EMPTY_LINE_HEIGHT,
    TRAIN_ID_HEIGHT,
    TRAIN_ORIENTATION_HEIGHT,
    TRAIN_LANDMARK_HEIGHT,
    TRAIN_DISTANCE_HEIGHT,
    TRAIN_ESTIMATED_VELOCITY_HEIGHT,
    TRAIN_STOPPING_DISTANCE_HEIGHT,
    TRAIN_MEASURED_VELOCITY_HEIGHT,
    TRAIN_ERROR_HEIGHT,
};

typedef struct DisplayData {
    int id;
    track_edge *edge;
    int distance;
    int velocity;
    int stopping_distance;
    int calibrated_velocity;
    int calibrated_error;
    enum TRAIN_ORIENTATION orientation;
    struct track_node *reserved_nodes[MAX_RESERVED_NODES];
} DisplayData;

static void train_orientation_update(int index, DisplayData *data, enum TRAIN_ORIENTATION orientation) {
    if (data->orientation == orientation) return;
    data->orientation = orientation;

    char command[128];
    char *pos = &command[0];

    pos += sprintf(pos, "\0337");
    int offset = strlen(ORIENTATION_STRING);
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_ORIENTATION_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1 + offset);
    char position[TRAIN_COLUMN_WIDTH];
    if (orientation == TRAIN_FORWARD) {
        sprintf(position, "F");
    } else if (orientation == TRAIN_BACKWARD) {
        sprintf(position, "B");
    } else {
        sprintf(position, "N/A");
    }
    pos += sputw(pos, TRAIN_COLUMN_WRITABLE_WIDTH - offset, ' ', position);
    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
}

static void train_edge_update(int index, DisplayData *data, track_edge *edge) {
    if (data->edge == edge) return;
    data->edge = edge;

    char command[128];
    char *pos = &command[0];

    pos += sprintf(pos, "\0337");
    int offset = strlen(LANDMARK_STRING);
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_LANDMARK_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1 + offset);
    char position[TRAIN_COLUMN_WIDTH];
    if (!edge) {
        sprintf(position, "N/A");
    } else {
        sprintf(position, "%s", edge->src->name);
    }
    pos += sputw(pos, TRAIN_COLUMN_WRITABLE_WIDTH - offset, ' ', position);
    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
}

static void train_distance_update(int index, DisplayData *data, int distance) {
    if (data->distance == distance) return;
    data->distance = distance;

    char command[128];
    char *pos = &command[0];

    pos += sprintf(pos, "\0337");
    int offset = strlen(DISTANCE_STRING);
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_DISTANCE_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1 + offset);
    char buf[TRAIN_COLUMN_WIDTH];
    sprintf(buf, "%dmm", distance / 1000);
    pos += sputw(pos, TRAIN_COLUMN_WRITABLE_WIDTH - offset, ' ', buf);
    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
}

static void train_estimated_velocity_update(int index, DisplayData *data, int velocity) {
    if (data->velocity == velocity) return;
    data->velocity = velocity;

    char command[128];
    char *pos = &command[0];

    pos += sprintf(pos, "\0337");
    int offset = strlen(ESTIMATED_SPEED_STRING);
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_ESTIMATED_VELOCITY_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1 + offset);
    char velocity_buf[TRAIN_COLUMN_WIDTH];
    sprintf(velocity_buf, "%dum/tick", velocity);
    pos += sputw(pos, TRAIN_COLUMN_WRITABLE_WIDTH - offset, ' ', velocity_buf);
    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
}

static void train_stopping_distance_update(int index, DisplayData *data, int stopping_distance) {
    if (data->stopping_distance == stopping_distance) return;
    data->stopping_distance = stopping_distance;

    char command[128];
    char *pos = &command[0];

    pos += sprintf(pos, "\0337");
    int offset = strlen(STOPPING_DISTANCE_STRING);
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_STOPPING_DISTANCE_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1 + offset);
    char stop_buf[TRAIN_COLUMN_WIDTH];
    sprintf(stop_buf, "%dmm", stopping_distance / 1000);
    pos += sputw(pos, TRAIN_COLUMN_WRITABLE_WIDTH - offset, ' ', stop_buf);
    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
}

static void train_calibrated_velocity_update(int index, DisplayData *data, int velocity) {
    if (data->calibrated_velocity == velocity) return;
    data->calibrated_velocity = velocity;

    char command[128];
    char *pos = &command[0];

    pos += sprintf(pos, "\0337");
    int offset = strlen(MEASURED_VELOCITY_STRING);
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_MEASURED_VELOCITY_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1 + offset);
    char buf[TRAIN_COLUMN_WIDTH];
    sprintf(buf, "%dum/tick", velocity);
    pos += sputw(pos, TRAIN_COLUMN_WRITABLE_WIDTH - offset, ' ', buf);
    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
}

static void train_error_update(int index, DisplayData *data, int error) {
    if (data->calibrated_error == error) return;
    data->calibrated_error = error;

    char command[128];
    char *pos = &command[0];

    pos += sprintf(pos, "\0337");
    int offset = strlen(ERROR_STRING);
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_ERROR_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1 + offset);
    char buf[TRAIN_COLUMN_WIDTH];
    sprintf(buf, "%dmm", error / 1000);
    pos += sputw(pos, TRAIN_COLUMN_WRITABLE_WIDTH - offset, ' ', buf);
    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
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

static void train_display_add(int index, DisplayData *display, int train) {
    display->id = train;

    char command[256];
    char *pos = &command[0];
    pos += sprintf(pos, "\0337");

    // Draw Trin Id.
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_ID_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1);
    pos += sprintf(pos, "Train: %d", train);

    // Draw Orientation.
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_ORIENTATION_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1);
    pos += sprintf(pos, ORIENTATION_STRING);

    // Draw Train Position
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_LANDMARK_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1);
    pos += sprintf(pos, LANDMARK_STRING);

    // Draw Train Distance 
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_DISTANCE_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1);
    pos += sprintf(pos, DISTANCE_STRING);

    // Draw Calibrated Train Velocity
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_ESTIMATED_VELOCITY_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1);
    pos += sprintf(pos, ESTIMATED_SPEED_STRING);

    // Draw Calibrated Train Stopping Distance
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_STOPPING_DISTANCE_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1);
    pos += sprintf(pos, STOPPING_DISTANCE_STRING);

    // Draw Train Speed
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_MEASURED_VELOCITY_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1);
    pos += sprintf(pos, MEASURED_VELOCITY_STRING);

    // Draw Train Error.
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_ERROR_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1);
    pos += sprintf(pos, ERROR_STRING);

    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
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
    Subscribe("LocationServerStream", PUBSUB_LOW);
    Subscribe("CalibrationServerStream", PUBSUB_LOW);

    // Initial Display.
    train_display_init();

    // Deal with out Subscription.
    int tid;
    struct Message msg, rply;
    int number_to_train[MAX_TRAINS];

    DisplayData train_displays[MAX_TRAINS];
    memset(train_displays, 0, sizeof(train_displays));

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
                int old_num_trains = num_trains;
                int index = get_train_index(number_to_train, &num_trains, data->id);
                DisplayData *display = &train_displays[index];
                if (old_num_trains < num_trains) {
                    train_display_add(index, display, data->id);
                }

                train_orientation_update(index, display, data->orientation);
                train_edge_update(index, display, data->edge);
                train_distance_update(index, display, data->distance);
                train_estimated_velocity_update(index, display, data->velocity);
                train_stopping_distance_update(index, display, data->stopping_distance);

                break;
            }
            case CALIBRATION_MESSAGE: {
                cuassert(msg.cs_msg.type == CALIBRATION_INFO, "Invalid Location Widget Request");

                // Ack Calibration Message.
                rply.type = CALIBRATION_MESSAGE;
                rply.cs_msg.type = CALIBRATION_INFO;
                Reply(tid, (char *) &rply, sizeof(rply));

                int old_num_trains = num_trains;
                int index = get_train_index(number_to_train, &num_trains, msg.cs_msg.train);
                DisplayData *display = &train_displays[index];
                if (old_num_trains < num_trains) {
                    train_display_add(index, display, msg.cs_msg.train);
                }

                train_calibrated_velocity_update(index, display, msg.cs_msg.velocity);
                train_error_update(index, display, msg.cs_msg.error);
                break;
            }
            default:
                cuassert(0, "Invalid Train Widget Message");
        }
    }

    Exit();
}
