#include <track.h>

#include <track_data.h>
#include <log.h>

static track_node track[TRACK_MAX];

int track_initialize(char track_name) {
    switch(track_name) {
        case 'A':
            init_tracka(track);
            break;
        case 'B':
            init_trackb(track);
            break;
        default:
            ulog("Invalid Track\n");
            break;
    }

    ulog("Initialized Track %c", track_name);

    return 0;
}

/**
 * Generate the next sensor on the track we can hit.
 */
int track_next_sensors(int node, struct track_node* sensors) {
    return 0;
}

int sensor_to_idx(char sensor, int num) {
   return (int)(sensor - 'A') * 16 + num;
}
