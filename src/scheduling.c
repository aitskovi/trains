/*
 * scheduling.c
 *
 *  Created on: May 22, 2013
 *      Author: aianus
 */

#include <scheduling.h>

#include <task.h>
#include <circular_queue.h>

static Task *last_of_priority[NUM_PRIORITIES];
static Task *head;

void initialize_scheduling () {
	unsigned int i;
	for (i = 0; i <  NUM_PRIORITIES; ++i) {
	    last_of_priority[i] = 0;
	}
	head = 0;
}

Task * schedule () {
    // TODO remove the check for head since we will always have at least one task in the lowest priority
    Task *result = head;
    if (head) {
        if (head == last_of_priority[head->priority]) {
            last_of_priority[head->priority] = 0;
        }
        if (head->next) {
            head = head->next;
            head->prev = 0;
        } else {
            head = 0;
        }
    }
    return result;
}

void make_ready(Task *task) {
    // Find the position in the skip list where this will go
    Task *previous  = last_of_priority[task->priority];
    unsigned int i;


    if (previous) {
        task->next = previous->next;
        task->prev = previous;
        previous->next = task;
    } else {
        task->next = head;
        if (head) {
            head->prev = task;
        }
        task->prev = 0;
        head = task;
    }


    for (i = task->priority; i < NUM_PRIORITIES; ++i) {
        if (last_of_priority[i] == task->prev) {
            last_of_priority[i] = task;
        }
    }
}
