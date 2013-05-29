/*
 * scheduling.c
 *
 *  Created on: May 22, 2013
 *      Author: aianus
 */

#include <scheduling.h>

#include <task.h>
#include <circular_queue.h>

static Task *queue_heads[NUM_PRIORITIES];
static Task *queue_tails[NUM_PRIORITIES];

void initialize_scheduling () {
	unsigned int i;
	for (i = 0; i <  NUM_PRIORITIES; ++i) {
        queue_heads[i] = 0;
        queue_tails[i] = 0;
	}
}

Task * schedule () {
    unsigned int i;
    Task *result;
    for (i = 0; i < NUM_PRIORITIES; ++i) {
        if ((result = queue_heads[i])) {
            queue_heads[i] = result->next;
            if (queue_tails[i] == result) queue_tails[i] = 0;
            result->state = ACTIVE;
            return result;
        }
    }
    return 0;
}

void make_ready(Task *task) {
    enum task_priority priority = task->priority;
    task->state = READY;

    if (queue_tails[priority] != 0) queue_tails[priority]->next = task;
    queue_tails[priority] = task;

    if (queue_heads[priority] == 0) queue_heads[priority] = task;
}
