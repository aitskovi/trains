#include <track.h>

#include <track_data.h>
#include <log.h>

#define SENSORS_PER_TYPE 16

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
    struct track_node **iterator = sensors;
    int i;
    for (i = 0; i < NUM_NODE_EDGES[node->type]; ++i) {
        struct track_edge *edge = &(node->edge[i]);
        struct track_node *child = edge->dest;
        if(!child) continue;

        if (child->type == NODE_SENSOR) {
            *iterator = child;
            iterator++;
        } else {
            int found = track_sensor_search(child, iterator);
            iterator += found;
        }
    }

    return iterator - sensors;
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

int idx_to_sensor(int idx, char *sensor, int *num) {
    *sensor = 'A' + (idx / SENSORS_PER_TYPE);
    *num = idx % 16 + 1;
    return 0;
}

int sensor_eq(track_node *sensor, char name, int num) {
    char name_b = 0;
    int num_b = 0;
    idx_to_sensor(sensor->num, &name_b, &num_b);
    return name == name_b && num == num_b;
}

track_node *track_get_sensor(char sensor, int num) {
    return &(track[sensor_to_idx(sensor, num)]);
}
