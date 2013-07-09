/*
 * heap_priority_queue.h
 *
 *  Created on: Jun 6, 2013
 *      Author: aianus
 */

#ifndef HEAP_PRIORITY_QUEUE_H_
#define HEAP_PRIORITY_QUEUE_H_

typedef struct PriorityQueueElement {
    unsigned int priority;
    void *data;
} PriorityQueueElement;

typedef struct HeapPriorityQueue {
    PriorityQueueElement *elements;
    unsigned int buffer_size;
    unsigned int pos;
} HeapPriorityQueue;

HeapPriorityQueue priority_queue_create(PriorityQueueElement *buffer, unsigned int size);
PriorityQueueElement priority_queue_peek(HeapPriorityQueue *queue);
PriorityQueueElement priority_queue_extract(HeapPriorityQueue *queue);
void priority_queue_insert(HeapPriorityQueue *queue, PriorityQueueElement element);
void priority_queue_delete(HeapPriorityQueue *queue, void *data);
unsigned int priority_queue_size(HeapPriorityQueue *queue);

#endif /* HEAP_PRIORITY_QUEUE_H_ */
