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

    return 0;
}

/**
 * Do a dfs until we hit all sensors after us.
 */
int track_sensor_search(struct track_node *node, struct track_node **sensors) {
    int i;
    int sensors_found = 0;
    for (i = 0; i < 2; ++i) {
        struct track_edge *edge = &node->edge[i];
        if (edge == 0) continue;

        struct track_node *child = edge->dest;

        if (child->type == NODE_SENSOR) {
            *sensors = child;
            sensors++;
            sensors_found++;
        } else {
            int found = track_sensor_search(node, sensors);
            sensors += found;
            sensors_found += found;
        }
    }

    return sensors_found;
}

/**
 * Generate the next sensor on the track we can hit.
 */
int track_next_sensors(int node, struct track_node** sensors) {
    track_node *tnode = &track[node];

    return track_sensor_search(tnode, sensors);
}

int sensor_to_idx(char sensor, int num) {
   return (int)(sensor - 'A') * 16 + (num - 1);
}
