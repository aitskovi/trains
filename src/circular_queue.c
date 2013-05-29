/*
 * circular_queue.c
 *
 *  Created on: May 22, 2013
 *      Author: aianus
 */


#include <circular_queue.h>

int circular_queue_empty(struct circular_queue *queue) {
    return queue->write_count == queue->read_count;
}

int circular_queue_push(struct circular_queue *queue, void *data) {
    queue->elements[queue->write_count++ % CIRCULAR_QUEUE_SIZE] = data;

   // Check if we've overrun our queue
   if ((queue->write_count - queue->read_count - 1) / CIRCULAR_QUEUE_SIZE > 0) {
       return -1;
   }

   return 0;
}

void *circular_queue_pop(struct circular_queue *queue) {
    if (!circular_queue_empty(queue)) {
        return queue->elements[queue->read_count++ % CIRCULAR_QUEUE_SIZE];
    } else {
    	return 0;
    }
}

void circular_queue_initialize(struct circular_queue *queue) {
    queue->read_count = 0;
    queue->write_count = 0;
}

unsigned int circular_queue_size(struct circular_queue *queue) {
    return queue->write_count - queue->read_count;
}
