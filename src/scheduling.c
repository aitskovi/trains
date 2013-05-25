/*
 * scheduling.c
 *
 *  Created on: May 22, 2013
 *      Author: aianus
 */

#include <scheduling.h>

#include <task.h>
#include <circular_queue.h>

static struct circular_queue ready_queues[NUM_PRIORITIES];
static struct circular_queue blocked;

void initialize_scheduling () {
	unsigned int i;
	for (i = 0; i <  NUM_PRIORITIES; ++i) {
		circular_queue_initialize(&ready_queues[i]);
	}
	circular_queue_initialize(&blocked);
}

Task * schedule () {
    unsigned int i;
    Task *result;
    for (i = 0; i < NUM_PRIORITIES; ++i) {
        if ((result = circular_queue_pop(&ready_queues[i]))) {
            return result;
        }
    }
    return 0;
}

void make_ready(Task *task) {
    circular_queue_push(&ready_queues[task->priority], task);
}
