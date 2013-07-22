#include <track.h>

#include <track_data.h>
#include <log.h>
#include <memory.h>
#include <string.h>
#include <switch.h>
#include <switch_server.h>

#define SENSORS_PER_TYPE 16
#define TRAIN_LENGTH 250000

static track_node track[TRACK_MAX];
static unsigned int REVERSE_PENALTY;
static unsigned int BLOCKED_TRACK_PENALTY;

int track_initialize(char track_name) {
    switch(track_name) {
        case 'A':
            init_tracka(track);
            break;
        case 'B':
            init_trackb(track);
            break;
        default:
            //ulog("Invalid Track\n");
            break;
    }

    // Default penalty for reversing and blocked track is ten, one hundred meter (basically only want to reverse/wait if impossible to find other route)
    REVERSE_PENALTY = 10000000;
    BLOCKED_TRACK_PENALTY = 100000000;
    //BLOCKED_TRACK_PENALTY = 0;
    return 0;
}

void track_set_reverse_penalty(unsigned int penalty) {
    REVERSE_PENALTY = penalty;
}

struct track_edge *track_next_edge(struct track_node *node) {
    int edge_direction = DIR_AHEAD;

    if (node->type == NODE_BRANCH) {
        edge_direction = switch_get_position(node->num) == STRAIGHT ? DIR_STRAIGHT : DIR_CURVED;
    }

    return &node->edge[edge_direction];
}

struct track_node *track_next_landmark(struct track_node *node) {
    if (!node) return 0;

    int edge_direction = DIR_AHEAD;

    if (node->type == NODE_BRANCH) {
        edge_direction = switch_get_position(node->num) == STRAIGHT ? DIR_STRAIGHT : DIR_CURVED;
    }

    return node->edge[edge_direction].dest;
}

struct track_node *track_previous_landmark(struct track_node *node) {
    if (!node) return 0;

    node = node->reverse;

    track_node *next_backward = track_next_landmark(node);

    if (!next_backward) return 0;

    return next_backward->reverse;
}

struct track_node *track_next_sensor(struct track_node *node) {
    if (!node) return 0;

    struct track_node *dest = track_next_landmark(node);
    while(dest && dest->type != NODE_SENSOR) {
        dest = track_next_landmark(dest);
    };
    return dest;
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

static track_node * get_closest_unvisited_node(struct dijkstra_data *dijkstra_state) {
    unsigned int result = 0;
    unsigned int result_distance = 0xFFFFFFFF;

    unsigned int i;
    for (i = 0; i < TRACK_MAX; ++i) {
        if (!dijkstra_state[i].visited && dijkstra_state[i].distance < result_distance) {
            result = i;
            result_distance = dijkstra_state[i].distance;
        }
    }

    if (dijkstra_state[result].visited || result_distance == 0xFFFFFFFF) {
        return 0;
    }

    return &track[result];
}

static int has_room_ahead_on_edge(track_edge *edge, int branch_allowed) {
    if (edge->dist > TRAIN_LENGTH) return 1;
    if (edge->dest) {
        switch (edge->dest->type) {
        case NODE_SENSOR:
            return 1;
        case NODE_BRANCH:
            return branch_allowed;
        case NODE_MERGE:
            return !branch_allowed;
        case NODE_EXIT:
            return 1;
        default:
            return 0;
        }
    }
    return 0;
}

static int has_room_ahead(track_node *node, int branch_allowed) {
    track_edge *forward = &node->edge[0];
    track_edge *forward2 = &node->edge[1];
    switch (node->type) {
    case NODE_BRANCH:
        return has_room_ahead_on_edge(forward, branch_allowed) && has_room_ahead_on_edge(forward2, branch_allowed);
    case NODE_NONE:
        return 0;
    default:
        return has_room_ahead_on_edge(forward, branch_allowed);
    }
    return 0;
}

static int has_room_behind(track_node *node) {
    return has_room_ahead(node->reverse, 0);
}

int can_reverse_at_node(track_node *node) {
    switch (node->type) {
    case NODE_EXIT:
        return 1;
    case NODE_SENSOR:
    case NODE_BRANCH:
        return (has_room_ahead(node, 1) && has_room_behind(node));
    }
    return 0;
}

// O(v^2) Dijkstra's
int calculate_path(int avoid_others, track_node *src, track_node *dest, track_node **path, unsigned int *path_length) {
    if (!src) {
        ulog("Calculate path called with null src");
        return 1;
    }

    if (!dest) {
        ulog("Calculate path called with null dest");
        return 1;
    }

    track_node **previous[TRACK_MAX];
    track_node **next[TRACK_MAX];
    struct dijkstra_data dijkstra_state[TRACK_MAX];
    memset(previous, 0, sizeof(previous));
    memset(next, 0, sizeof(next));
    memset(dijkstra_state, 0, sizeof(dijkstra_state));

    track_node *current, *neighbour;

    // Configure distance to be +inf
    unsigned int i;
    for (i = 0; i < TRACK_MAX; ++i) {
        dijkstra_state[i].distance = 0xFFFFFFFF;
        dijkstra_state[i].visited = 0;
    }

    dijkstra_state[src - track].distance = 0;

    while ((current = get_closest_unvisited_node(dijkstra_state))) {
        // ulog("\nDijkstra's on node %s", current->name);
        dijkstra_state[current - track].visited = 1;
        unsigned int j;
        // For each neighbour
        for (j = 0; j < NUM_NODE_EDGES[current->type]; ++j) {
            neighbour = current->edge[j].dest;
            if (neighbour && neighbour->type != NODE_NONE && !dijkstra_state[neighbour - track].visited) {
                unsigned int dist = dijkstra_state[current - track].distance + current->edge[j].dist;
                if (avoid_others && neighbour->owner) {
                    dist += BLOCKED_TRACK_PENALTY;
                }
                if (dist < dijkstra_state[neighbour - track].distance) {
                    dijkstra_state[neighbour - track].distance = dist;
                    previous[neighbour - track] = current;
                }
            }
        }

        // If there is enough space around current node we could also reverse
        if (can_reverse_at_node(current)) {
            neighbour = current->reverse;
            if (neighbour && neighbour->type != NODE_NONE && !dijkstra_state[neighbour - track].visited) {
                unsigned int dist = dijkstra_state[current - track].distance + REVERSE_PENALTY;
                if (avoid_others && neighbour->owner) {
                    dist += BLOCKED_TRACK_PENALTY;
                }
                if (dist < dijkstra_state[neighbour - track].distance) {
                    dijkstra_state[neighbour - track].distance = dist;
                    previous[neighbour - track] = current;
                }
            }
        }


        if (current == dest) {
            break;
        }
    }

    if (dijkstra_state[dest - track].distance == 0xFFFFFFFF) {
        return 1;
    }

    // Reverse the linked list we built
    current = dest;
    while (src != previous[current - track]) {
        track_node *prev = previous[current - track];
        next[prev - track] = current;
        current = prev;
    }
    next[src - track] = current;

    // Output to client
    *path_length = 0;
    current = src;
    do {
        path[(*path_length)++] = current;
        current = next[current - track];
    } while (current != dest);
    path[(*path_length)++] = dest;

    return 0;
}

// Returns distance from node1 to node2 going forwards
int distance_between_nodes(track_node *src, track_node *dest) {
    if (!src) {
        ulog("Distance between nodes called with null src");
        return -1;
    }

    if (!dest) {
        ulog("Distance between nodes called with null dest");
        return -1;
    }

    struct dijkstra_data dijkstra_state[TRACK_MAX];
    memset(dijkstra_state, 0, sizeof(dijkstra_state));

    track_node *current, *neighbour;

    // Configure distance to be +inf
    unsigned int i;
    for (i = 0; i < TRACK_MAX; ++i) {
        dijkstra_state[i].distance = 0xFFFFFFFF;
        dijkstra_state[i].visited = 0;
    }

    dijkstra_state[src - track].distance = 0;

    while ((current = get_closest_unvisited_node(dijkstra_state))) {
        // ulog("\nDijkstra's on node %s", current->name);
        dijkstra_state[current - track].visited = 1;
        unsigned int j;
        // For each neighbour
        for (j = 0; j < NUM_NODE_EDGES[current->type]; ++j) {
            neighbour = current->edge[j].dest;
            if (neighbour && neighbour->type != NODE_NONE && !dijkstra_state[neighbour - track].visited) {
                unsigned int dist = dijkstra_state[current - track].distance + current->edge[j].dist;
                if (dist < dijkstra_state[neighbour - track].distance) {
                    dijkstra_state[neighbour - track].distance = dist;
                }
            }
        }

        if (current == dest) {
            break;
        }
    }

    if (dijkstra_state[dest - track].distance == 0xFFFFFFFF) {
        return -1;
    } else {
        return dijkstra_state[dest - track].distance;
    }

    return -1;
}

// Returns if node2 is logically ahead of node1 (quicker to go node1 -> node2 than node2->node1)
int is_node_ahead_of_node(track_node *node1, track_node *node2) {
    int node1tonode2 = distance_between_nodes(node1, node2);
    if (node1tonode2 < 0) {
        return 0;
    }
    int node2tonode1 = distance_between_nodes(node2, node1);
    if (node2tonode1 < 0) {
        return 1;
    }
    return node1tonode2 < node2tonode1;
}
