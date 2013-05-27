/*
 * scheduling.h
 *
 *  Created on: May 22, 2013
 *      Author: aianus
 */

#ifndef SCHEDULING_H_
#define SCHEDULING_H_

struct Task;

enum task_priority {
	REALTIME,
	HIGH,
	MEDIUM,
	LOW,
	NUM_PRIORITIES
};

enum task_state {
    UNKNOWN,
    READY,
    ACTIVE,
    SEND_BLOCKED,
    RPLY_BLOCKED,
    RECV_BLOCKED,
};

struct Task *schedule();
void make_ready(struct Task *task);
void initialize_scheduling();

#endif /* SCHEDULING_H_ */
