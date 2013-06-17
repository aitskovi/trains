/*
 * priority_queue_tests.c
 *
 *  Created on: Jun 8, 2013
 *      Author: aianus
 */

#include <heap_priority_queue.h>
#include <log.h>

#include <verify.h>

int equal_element(PriorityQueueElement a, PriorityQueueElement b) {
    return a.data == b.data && a.priority == b.priority;
}

int priority_queue_operations_test() {
    // Can store 9 elements
    PriorityQueueElement elements[9];
    PriorityQueueElement buffer[10];

    HeapPriorityQueue queue = priority_queue_create(buffer, 10);
    vassert(!priority_queue_size(&queue));

    int i;
    for (i = 0; i < 9; ++i) {
        elements[i].priority = i;
        elements[i].data = 0xAAAAAAA0 + i;
    }

    priority_queue_insert(&queue, elements[6]);
    vassert(equal_element(elements[6], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[8]);
    vassert(equal_element(elements[6], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[3]);
    vassert(equal_element(elements[3], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[1]);
    vassert(equal_element(elements[1], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[4]);
    vassert(equal_element(elements[1], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[5]);
    vassert(equal_element(elements[1], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[0]);
    vassert(equal_element(elements[0], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[7]);
    vassert(equal_element(elements[0], priority_queue_peek(&queue)));
    priority_queue_insert(&queue, elements[2]);
    vassert(equal_element(elements[0], priority_queue_peek(&queue)));

    vassert(9 == priority_queue_size(&queue));

    for (i = 0; i < 9; ++i) {
        PriorityQueueElement element = priority_queue_extract(&queue);
        dlog("Expecting element %u, found %u\n", i, element.priority);
        vassert(equal_element(elements[i], element));
    }

    return 0;
}

struct vsuite *priority_queue_suite() {
    struct vsuite *suite = vsuite_create("Priority Queue Tests", 0);
    vsuite_add_test(suite, priority_queue_operations_test);
    return suite;
}
