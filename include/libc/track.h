#ifndef _TRACK_H_
#define _TRACK_H_

#include <track_node.h>

#define TRACK_A 'A'
#define TRACK_B 'B'

int track_initialize(char track);
void track_set_reverse_penalty(unsigned int penalty);
/**
 * Generate the next sensor on the track we can hit.
 */
int track_next_sensors(int node, struct track_node** sensors);

/**
 * Do a dfs until we hit all sensors after us.
 */
int track_sensor_search(struct track_node *node, struct track_node **sensors);

int sensor_to_idx(char sensor, int num);

track_node *track_get_sensor(char sensor, int num);
int sensor_eq(track_node *sensor, char name, int num);

struct dijkstra_data {
    unsigned int distance;
    int visited;
};

track_node *track_get_by_name(char * name);
int can_reverse_at_node(track_node *node);
int calculate_path(track_node *src, track_node *dest, track_node **path, unsigned int *path_length);

struct track_edge *track_next_edge(struct track_node *node);
struct track_node *track_next_landmark(struct track_node *node);
struct track_node *track_previous_landmark(struct track_node *node);
struct track_node *track_next_sensor(struct track_node *node);
int is_node_ahead_of_node(track_node *node1, track_node *node2);

#endif
