#include <track.h>

#include <track_data.h>
#include <log.h>
#include <memory.h>
#include <string.h>
#include <switch_server.h>

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

track_node *track_get_by_name(char * name) {
    unsigned int i;
    for (i = 0; i < TRACK_MAX; ++i) {
        if (streq(track[i].name, name)) {
            return &track[i];
        }
    }

    return 0;
}

static track_node * get_closest_unvisited_node() {
    unsigned int result = 0;
    unsigned int result_distance = 0xFFFFFFFF;

    unsigned int i;
    for (i = 0; i < TRACK_MAX; ++i) {
        if (!track[i].visited && track[i].distance < result_distance) {
            result = i;
            result_distance = track[i].distance;
        }
    }

    if (track[result].visited || result_distance == 0xFFFFFFFF) {
        return 0;
    }

    return &track[result];
}

// O(v^2) Dijkstra's
int configure_track_for_path(track_node *src, track_node *dest) {
    track_node **previous[TRACK_MAX];
    memset(previous, 0, sizeof(previous));

    track_node *current;

    // Configure distance to be +inf
    unsigned int i;
    for (i = 0; i < TRACK_MAX; ++i) {
        track[i].distance = 0xFFFFFFFF;
        track[i].visited = 0;
    }

    src->distance = 0;

    while ((current = get_closest_unvisited_node())) {
        // ulog("\nDijkstra's on node %s", current->name);
        current->visited = 1;
        unsigned int j;
        // For each neighbour
        for (j = 0; j < NUM_NODE_EDGES[current->type]; ++j) {
            track_node *neighbour = current->edge[j].dest;
            if (neighbour && neighbour->type != NODE_NONE && !neighbour->visited) {
                unsigned int dist = current->distance + current->edge[j].dist;
                if (dist < neighbour->distance) {
                    neighbour->distance = dist;
                    previous[neighbour - track] = current;
                }
            }
        }
        /*
        if (current == dest) {
            ulog("\nDijsktras found dest");
            break;
        }
        */
    }

    ulog("\nDijsktras finished");

    if (dest->distance == 0xFFFFFFFF) {
        return 1;
    }

    // Path may not exist since we are not supporting reverse atm
    current = dest;
    while (src != previous[current - track]) {
        track_node *prev = previous[current - track];
        if (!prev) {
            ulog("\nInvalid prev pointer");
            break;
        }
        if (prev->type == NODE_BRANCH) {
            if (prev->edge[DIR_STRAIGHT].dest == current) {
                SetSwitch(prev->num, STRAIGHT);
            } else if (prev->edge[DIR_CURVED].dest == current) {
                SetSwitch(prev->num, CURVED);
            } else {
                ulog("\nDijsktra path was invalid");
            }
        }
        current = prev;
    }

    return 0;

}
