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

#define REFRESH_RATE 5

#define TRAIN_TABLE_HEIGHT 9
#define TRAIN_COLUMN_WIDTH 60
#define TRAIN_COLUMN_WRITABLE_WIDTH TRAIN_COLUMN_WIDTH - 1
#define MAX_RESERVED_NODES 14

#define TRAIN_STRING "Train:"
#define LANDMARK_STRING "Landmark:"
#define DISTANCE_STRING "Distance:"
#define ESTIMATED_SPEED_STRING "Est. Speed:"
#define STOPPING_DISTANCE_STRING "Stop Dist.:"
#define MEASURED_VELOCITY_STRING "Mea. Speed:"
#define ERROR_STRING "Error:"
#define RESERVED_NODES_STRING "Reserved:"
#define ORIENTATION_STRING "Orientation:"
#define DESTINATION_STRING "Destination:"

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
    TRAIN_RESERVED_NODES_HEIGHT,
    TRAIN_DESTINATION_HEIGHT
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
    track_node *reserved_nodes[MAX_RESERVED_NODES];
    track_node *destination;
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

static void train_destination_update(int index, DisplayData *data, track_node *destination) {
    if (data->destination == destination) return;
    data->destination = destination;

    char command[256];
    char *pos = &command[0];

    char buf[TRAIN_COLUMN_WIDTH];
    char *buf_pos = &buf[0];

    pos += sprintf(pos, "\0337");
    int offset = strlen(DESTINATION_STRING);
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_DESTINATION_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1 + offset);

    if (data->destination) {
        buf_pos += sprintf(buf_pos, "%s", data->destination->name);
    } else {
        buf_pos += sprintf(buf_pos, "%s", "None");
    }

    pos += sputw(pos, TRAIN_COLUMN_WRITABLE_WIDTH - offset, ' ', buf);
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

static void train_reserved_node_update(int index, DisplayData *data) {
    char command[256];
    char *pos = &command[0];

    char buf[TRAIN_COLUMN_WIDTH];
    char *buf_pos = &buf[0];

    pos += sprintf(pos, "\0337");
    int offset = strlen(RESERVED_NODES_STRING);
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_RESERVED_NODES_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1 + offset);

    unsigned int j;
    for (j = 0; j < MAX_RESERVED_NODES; ++j) {
        if (data->reserved_nodes[j] != 0) {
            buf_pos += sprintf(buf_pos, "%s ", data->reserved_nodes[j]->name);
        }
    }

    pos += sputw(pos, TRAIN_COLUMN_WRITABLE_WIDTH - offset, ' ', buf);
    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
}

static void train_reserved_node_released(int index, DisplayData *data, track_node *node) {
    ulog("Train widget got released event for index %d, train %d, node %s", index, data->id, node->name);
    unsigned int j;
    for (j = 0; j < MAX_RESERVED_NODES; ++j) {
        if (data->reserved_nodes[j] == node) {
            data->reserved_nodes[j] = 0;
            train_reserved_node_update(index, data);
            return;
        }
    }
    ulog("ERROR: Train widget got release event for unreserved node");
}

static void train_reserved_node_reserved(int index, DisplayData *data, track_node *node) {
    unsigned int j;
    for (j = 0; j < MAX_RESERVED_NODES; ++j) {
        if (data->reserved_nodes[j] == 0) {
            data->reserved_nodes[j] = node;
            train_reserved_node_update(index, data);
            return;
        }
    }
    ulog("ERROR: Train widget: Reserved more than MAX_RESERVED_NODES");
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

    // Draw Train Reservations.
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_RESERVED_NODES_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1);
    pos += sprintf(pos, RESERVED_NODES_STRING);

    // Draw Train Destination
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_DESTINATION_HEIGHT, index * TRAIN_COLUMN_WIDTH + 1);
    pos += sprintf(pos, DESTINATION_STRING);

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

void train_widget_notifier() {
    tid_t train_widget = MyParentTid();

    for (;;) {
        Message msg, rply;
        msg.type = TRAIN_DISPLAY_MESSAGE;
        Send(train_widget, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
        cuassert(rply.type == TRAIN_DISPLAY_MESSAGE, "Invalid Reply");

        Delay(REFRESH_RATE);
    }

    Exit();
}

void train_widget() {
    RegisterAs("TrainWidget");

    // Find the location server and subscribe.
    Subscribe("LocationServerStream", PUBSUB_LOW);
    Subscribe("CalibrationServerStream", PUBSUB_LOW);
    Subscribe("ReservationServerStream", PUBSUB_LOW);

    Create(HIGHEST, train_widget_notifier);

    // Initial Display.
    train_display_init();

    // Deal with out Subscription.
    int tid;
    struct Message msg, rply;
    int number_to_train[MAX_TRAINS];

    DisplayData old_train_displays[MAX_TRAINS];
    memset(old_train_displays, 0, sizeof(old_train_displays));
    DisplayData new_train_displays[MAX_TRAINS];
    memset(new_train_displays, 0, sizeof(new_train_displays));


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
                DisplayData *display = &new_train_displays[index];
                if (old_num_trains < num_trains) {
                    train_display_add(index, display, data->id);
                }

                display->orientation = data->orientation;
                display->edge = data->edge;
                display->distance = data->distance;
                display->velocity = data->velocity;
                display->stopping_distance = data->stopping_distance;
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
                DisplayData *display = &new_train_displays[index];
                if (old_num_trains < num_trains) {
                    train_display_add(index, display, msg.cs_msg.train);
                }

                display->calibrated_velocity = msg.cs_msg.velocity;
                display->calibrated_error = msg.cs_msg.error;
                break;
            }
            case TRAIN_DISPLAY_MESSAGE: {
                // Ack Calibration Message.
                rply.type = TRAIN_DISPLAY_MESSAGE;
                Reply(tid, (char *) &rply, sizeof(rply));

                int index;
                for (index = 0; index < num_trains; ++index) {
                    DisplayData *new_display = &new_train_displays[index];
                    DisplayData *old_display = &old_train_displays[index];
                    train_orientation_update(index, old_display, new_display->orientation);
                    train_edge_update(index, old_display, new_display->edge);
                    train_distance_update(index, old_display, new_display->distance);
                    train_estimated_velocity_update(index, old_display, new_display->velocity);
                    train_stopping_distance_update(index, old_display, new_display->stopping_distance);
                    train_calibrated_velocity_update(index, old_display, new_display->calibrated_velocity);
                    train_error_update(index, old_display, new_display->calibrated_error);
                }
                break;
            }
            case TRAIN_MESSAGE: {
                cuassert(msg.tr_msg.type == COMMAND_GOTO, "Invalid Location Widget Request from train");

                // Ack
                rply.type = TRAIN_MESSAGE;
                rply.tr_msg.type = COMMAND_ACKNOWLEDGED;
                Reply(tid, (char *) &rply, sizeof(rply));

                int old_num_trains = num_trains;
                int index = get_train_index(number_to_train, &num_trains, msg.tr_msg.train);
                DisplayData *display = &train_displays[index];
                if (old_num_trains < num_trains) {
                    train_display_add(index, display, msg.tr_msg.train);
                }

                train_destination_update(index, display, msg.tr_msg.destination);
                break;
            }
            case RESERVATION_SERVER_MESSAGE: {
                rply.type = RESERVATION_SERVER_MESSAGE;
                rply.rs_msg.type = RESERVATION_SUCCESS_RESPONSE;
                Reply(tid, (char *) &rply, sizeof(rply));

                int old_num_trains = num_trains;
                int index = get_train_index(number_to_train, &num_trains, msg.rs_msg.train_no);
                DisplayData *display = &train_displays[index];
                if (old_num_trains < num_trains) {
                    train_display_add(index, display, msg.rs_msg.train_no);
                }

                switch (msg.rs_msg.type) {
                    case RESERVATION_RESERVE:
                        train_reserved_node_reserved(index, display, msg.rs_msg.node);
                        break;
                    case RESERVATION_RELEASE:
                        train_reserved_node_released(index, display, msg.rs_msg.node);
                        break;
                    default:
                        ulog("Train Widget: Invalid Reservation Server Message");
                }

                break;
            }
            default:
                cuassert(0, "Invalid Train Widget Message");
        }
    }

    Exit();
}
