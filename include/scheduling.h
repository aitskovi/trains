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
	HIGHEST,
	HIGHER,
	HIGH,
	MEDIUM,
	LOW,
	LOWER,
	LOWEST,
	NUM_PRIORITIES
};

enum task_state {
    UNKNOWN,
    READY,
    ACTIVE,
    SEND_BLOCKED,
    RPLY_BLOCKED,
    RECV_BLOCKED,
    EVT_BLOCKED,
    ZOMBIE,
};

struct Task *schedule();
void make_ready(struct Task *task);
void initialize_scheduling();

#endif /* SCHEDULING_H_ */
