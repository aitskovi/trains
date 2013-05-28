#include <circular_queue.h>
#include <assert.h>

void test_queue_initialization() {
    struct circular_queue queue;
    circular_queue_initialize(&queue);

    assert(queue.write_count == 0);
    assert(queue.read_count == 0);
}

void test_queue_operations() {
    struct circular_queue queue;
    circular_queue_initialize(&queue);

    // Verify the queue works for one element.
    int result = circular_queue_push(&queue, (void *)1);
    assert(result == 0);
    void *data = circular_queue_pop(&queue);
    assert(data == 1);

    // Verify the queue is fifo.
    result = circular_queue_push(&queue, (void *)2);
    assert(result == 0);
    result = circular_queue_push(&queue, (void *)1);
    assert(result == 0);
    data = circular_queue_pop(&queue);
    assert(data == 2);
}

void test_queue_empty() {
    struct circular_queue queue;
    circular_queue_initialize(&queue);

    void *data = circular_queue_pop(&queue);
    assert(data == 0);
    assert(circular_queue_empty(&queue));

    int i;
    for (i = 0; i < CIRCULAR_QUEUE_SIZE; ++i) {
        assert(circular_queue_push(&queue, (void *)1) == 0);
    }
    for (i = 0; i < CIRCULAR_QUEUE_SIZE; ++i) {
        assert(circular_queue_pop(&queue) != 0);
    }
    data = circular_queue_pop(&queue);
    assert(data == 0);
    assert(circular_queue_empty(&queue));
}

void test_queue_full() {
    struct circular_queue queue;
    circular_queue_initialize(&queue);

    int i;
    for (i = 0; i < CIRCULAR_QUEUE_SIZE; ++i) {
        assert(circular_queue_push(&queue, (void *)1) == 0);
    }
    assert(circular_queue_push(&queue, (void *)1) == -1);
}

int main() {
    test_queue_initialization();
    test_queue_operations();
    test_queue_empty();
    test_queue_full();
    return 0;    
}
