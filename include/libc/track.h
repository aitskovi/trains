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

int sensor_to_idx(char sensor, int num);

#endif
