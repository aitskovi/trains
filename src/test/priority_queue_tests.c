/*
 * priority_queue_tests.c
 *
 *  Created on: Jun 8, 2013
 *      Author: aianus
 */

#include <heap_priority_queue.h>
#include <log.h>
#include <assert.h>

int equal_element(PriorityQueueElement a, PriorityQueueElement b) {
    return a.data == b.data && a.priority == b.priority;
}

int main() {
    // Can store 9 elements
    PriorityQueueElement elements[9];
    PriorityQueueElement buffer[10];

    HeapPriorityQueue queue = priority_queue_create(buffer, 10);
    assert(!priority_queue_size(&queue));

    int i;
    for (i = 0; i < 9; ++i) {
        elements[i].priority = i;
        elements[i].data = 0xAAAAAAA0 + i;
    }

    priority_queue_insert(&queue, elements[6]);
    assert(equal_element(elements[6], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[8]);
    assert(equal_element(elements[6], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[3]);
    assert(equal_element(elements[3], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[1]);
    assert(equal_element(elements[1], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[4]);
    assert(equal_element(elements[1], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[5]);
    assert(equal_element(elements[1], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[0]);
    assert(equal_element(elements[0], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[7]);
    assert(equal_element(elements[0], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[2]);
    assert(equal_element(elements[0], priority_queue_peek(&queue)));

    assert(9 == priority_queue_size(&queue));

    for (i = 0; i < 9; ++i) {
        PriorityQueueElement element = priority_queue_extract(&queue);
        log("Expecting element %u, found %u\n", i, element.priority);
        assert(equal_element(elements[i], element));
    }
}
