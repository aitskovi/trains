#ifndef _TRACK_H_
#define _TRACK_H_

#include <track_node.h>

#define TRACK_A 'A'
#define TRACK_B 'B'

int track_initialize(char track);

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


#endif
