/*
 * clock_server.h
 *
 *  Created on: Jun 8, 2013
 *      Author: aianus
 */

#ifndef CLOCK_SERVER_H_
#define CLOCK_SERVER_H_

#include <clock.h>
#include <task.h>

#define MAX_BLOCKED_TASKS MAX_TASKS

int Time();
int DelayUntil(int time);
int Delay(int ticks);

void clock_server();

#endif /* CLOCK_SERVER_H_ */
