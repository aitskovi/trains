#ifndef _CALIBRATION_H_
#define _CALIBRATION_H_

#include <track_node.h>

int velocity(int train, int speed, track_edge *edge);

int ticks_for_acceleration(int train, int start_speed, int end_speed);
int distance_for_tick(int train, int tick, int start_speed, int end_speed);
int timeout_for_speed(int train, int speed);
int stopping_distance_for_speed(int train, int speed);

void calibration_server();

#endif
