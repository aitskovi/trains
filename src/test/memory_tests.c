#include <assert.h>
#include <memory.h>

void reset() {
    initialize_memory();
}

void test_kmalloc_basic() {
    reset();

    int *data = kmalloc(sizeof(int));
    assert(data != 0);
}

void test_kmalloc_full() {
    reset();

    int *data = kmalloc(HEAP_SIZE);
    assert(data != 0);
}

void test_kmalloc_invalid() {
    reset();

    void *data = kmalloc(HEAP_SIZE + 1);
    assert(data == 0);
}

int main() {
    test_kmalloc_basic();
    test_kmalloc_full();
    test_kmalloc_invalid();
    return 0;
}
