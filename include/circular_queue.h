/*
 * circular_queue.h
 *
 *  Created on: May 22, 2013
 *      Author: aianus
 */

#ifndef CIRCULAR_QUEUE_H_
#define CIRCULAR_QUEUE_H_

// NOTE this must be a power of 2
#define CIRCULAR_QUEUE_SIZE 8 // TODO write kmalloc/kfree, this is stupid

struct circular_queue {
	void *elements[CIRCULAR_QUEUE_SIZE];
	unsigned int write_count;
	unsigned int read_count;
};

int circular_queue_push(struct circular_queue *queue, void *data);
void *circular_queue_pop(struct circular_queue *queue);
void circular_queue_initialize(struct circular_queue *queue);

#endif /* CIRCULAR_QUEUE_H_ */
