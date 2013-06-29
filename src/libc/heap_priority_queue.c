/*
 * heap_priority_queue.c
 *
 *  Created on: Jun 8, 2013
 *      Author: aianus
 */

#include <heap_priority_queue.h>
#include <log.h>

void dump_elements (HeapPriorityQueue *queue) {
    int i;
    unsigned int size = priority_queue_size(queue);
    for (i = 1; i <= size; ++i) {
        log("%u ", queue->elements[i].priority);
    }
    log("\n");
}

void swap (PriorityQueueElement *a, PriorityQueueElement *b) {
    PriorityQueueElement tmp;
    tmp = *a;
    *a = *b;
    *b = tmp;
}

void bubble_up(HeapPriorityQueue *queue, unsigned int pos) {
    if (pos == 0 || pos == 1) {
        return;
    }
    PriorityQueueElement *us = &queue->elements[pos];
    PriorityQueueElement *parent = &queue->elements[pos/2];
    if (parent->priority > us->priority) {
        swap(us, parent);
        bubble_up(queue, pos/2);
    }
}

void bubble_down(HeapPriorityQueue *queue, unsigned int pos) {
    if (pos >= (queue->pos - 1)) {
        return;
    }
    PriorityQueueElement *us = &queue->elements[pos];
    PriorityQueueElement *child1 = (pos*2 < queue->pos) ? &queue->elements[pos*2] : 0;
    PriorityQueueElement *child2 = (pos*2 + 1 < queue->pos) ? &queue->elements[pos*2 + 1] : 0;
    unsigned int min_child_pos = 0;
    if (child1) {
        min_child_pos = pos * 2;
        if (child2) {
            if (child2->priority < child1->priority) {
                min_child_pos = pos * 2 + 1;
            }
        }
    }
    if (min_child_pos && queue->elements[min_child_pos].priority < us->priority) {
        swap(us, &queue->elements[min_child_pos]);
        bubble_down(queue, min_child_pos);
    }
}

HeapPriorityQueue priority_queue_create(PriorityQueueElement *buffer, unsigned int buffer_size) {
    HeapPriorityQueue result;
    result.elements = buffer;
    result.buffer_size = buffer_size;
    result.pos = 1;

    return result;
}

PriorityQueueElement priority_queue_peek(HeapPriorityQueue *queue) {
    // TODO assert priority_queue_size() > 0
    return queue->elements[1];
}

PriorityQueueElement priority_queue_extract(HeapPriorityQueue *queue) {
    // TODO assert priority_queue_size() > 0
    PriorityQueueElement result = queue->elements[1];

    queue->elements[1] = queue->elements[queue->pos - 1];
    queue->pos--;
    bubble_down(queue, 1);

    return result;
}

void priority_queue_insert(HeapPriorityQueue *queue, PriorityQueueElement element) {
    // TODO assert priority_queue_size() < buffer_size
    queue->elements[queue->pos] = element;
    bubble_up(queue, queue->pos);
    queue->pos++;
}

unsigned int priority_queue_size(HeapPriorityQueue *queue) {
    return queue->pos - 1;
}
