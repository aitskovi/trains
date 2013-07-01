#include <train_widget.h>

#include <sprintf.h>
#include <write_server.h>
#include <location_service.h>
#include <track_node.h>

#define TRAIN_TABLE_HEIGHT 9
#define TRAIN_COLUMN_WIDTH 10

/**
 * Update a specific train.
 */
int train_display_update(int index, struct TrainLocation *train) {
    char command[128];
    char *pos = &command[0];

    // Draw Train:
    pos += sprintf(pos, "\0337\033[%u;%uH", TRAIN_TABLE_HEIGHT, 1);
    pos += sprintf(pos, "Trains:");

    // Draw Train Number.
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_TABLE_HEIGHT + 1, index * TRAIN_COLUMN_WIDTH + 1);
    char num[10];
    sprintf(num, "Train %d", train->number);
    pos += sputw(pos, TRAIN_COLUMN_WIDTH, ' ', num);

    // Draw Train Position
    pos += sprintf(pos, "\033[%u;%uH", TRAIN_TABLE_HEIGHT + 2, index * TRAIN_COLUMN_WIDTH + 1);
    char position[TRAIN_COLUMN_WIDTH];
    if (!train->landmark) {
        sprintf(position, "N/A");
    } else {
        sprintf(position, "%s %dcm", train->landmark->name, train->distance);
    }
    pos += sputw(pos, TRAIN_COLUMN_WIDTH, ' ', position);
    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);

    return 0;
}

int train_display_init() {
    /*
    char command[128];
    char *pos = &command[0];
    pos += sprintf(pos, "\0337\033[%u;%uH", TRAIN_TABLE_HEIGHT, 1);
    pos += sprintf(pos, "Trains:");
    Write(COM2, command, pos - command);
    return 0;
    */
}
