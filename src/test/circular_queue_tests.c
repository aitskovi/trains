#include <circular_queue.h>

#include <verify.h>

int test_queue_initialization() {
    struct circular_queue queue;
    circular_queue_initialize(&queue);

    vassert(queue.write_count == 0);
    vassert(queue.read_count == 0);

    return 0;
}

int test_queue_operations() {
    struct circular_queue queue;
    circular_queue_initialize(&queue);

    // Verify the queue works for one element.
    int result = circular_queue_push(&queue, (void *)1);
    vassert(result == 0);
    void *data = circular_queue_pop(&queue);
    vassert(data == 1);

    // Verify the queue is fifo.
    result = circular_queue_push(&queue, (void *)2);
    vassert(result == 0);
    result = circular_queue_push(&queue, (void *)1);
    vassert(result == 0);
    data = circular_queue_pop(&queue);
    vassert(data == 2);

    return 0;
}

int test_queue_empty() {
    struct circular_queue queue;
    circular_queue_initialize(&queue);

    void *data = circular_queue_pop(&queue);
    vassert(data == 0);
    vassert(circular_queue_empty(&queue));

    int i;
    for (i = 0; i < CIRCULAR_QUEUE_SIZE; ++i) {
        vassert(circular_queue_push(&queue, (void *)1) == 0);
    }
    for (i = 0; i < CIRCULAR_QUEUE_SIZE; ++i) {
        vassert(circular_queue_pop(&queue) != 0);
    }
    data = circular_queue_pop(&queue);
    vassert(data == 0);
    vassert(circular_queue_empty(&queue));

    return 0;
}

int test_queue_full() {
    struct circular_queue queue;
    circular_queue_initialize(&queue);

    int i;
    for (i = 0; i < CIRCULAR_QUEUE_SIZE; ++i) {
        vassert(circular_queue_push(&queue, (void *)1) == 0);
    }
    vassert(circular_queue_push(&queue, (void *)1) == -1);

    return 0;
}

struct vsuite* circular_queue_suite() {
    struct vsuite *suite = vsuite_create("Circular Queue Tests", 0);
    vsuite_add_test(suite, test_queue_initialization);
    vsuite_add_test(suite, test_queue_operations);
    vsuite_add_test(suite, test_queue_empty);
    vsuite_add_test(suite, test_queue_full);
    return suite;
}
