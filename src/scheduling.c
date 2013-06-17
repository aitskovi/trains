/*
 * scheduling.c
 *
 *  Created on: May 22, 2013
 *      Author: aianus
 */

#include <scheduling.h>

#include <bits.h>
#include <task.h>
#include <circular_queue.h>

static int available;
static Task *queue_heads[NUM_PRIORITIES];
static Task *queue_tails[NUM_PRIORITIES];

void initialize_scheduling () {
	unsigned int i;
	for (i = 0; i <  NUM_PRIORITIES; ++i) {
        queue_heads[i] = 0;
        queue_tails[i] = 0;
	}
    available = 0;
}

Task * schedule () {
    unsigned int i;
    Task *result;

    int priority = ffs(available) - 1;

    if (priority < 0) return 0;

    result = queue_heads[priority];
    if (queue_tails[priority] == result) {
        queue_tails[priority] = 0;
        available &= ~(1 << priority);
    }
    queue_heads[priority] = result->next;
    result->state = ACTIVE;
    result->next = 0;
    return result;
}

void make_ready(Task *task) {
    enum task_priority priority = task->priority;
    task->state = READY;

    // If we have someone in the list, point them to us.
    if (queue_tails[priority] != 0) queue_tails[priority]->next = task;

    // If no one is in the list, we're the new head!
    if (queue_heads[priority] == 0) {
        queue_heads[priority] = task;
        available |= (1 << priority);
    }

    // We're always the new tail.
    queue_tails[priority] = task;
}
