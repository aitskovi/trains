#include <memory.h>

#include <verify.h>

void memory_reset() {
    initialize_memory();
}

int test_kmalloc_basic() {
    int *data = kmalloc(sizeof(int));
    vassert(data != 0);

    return 0;
}

int test_kmalloc_full() {
    int *data = kmalloc(HEAP_SIZE);
    vassert(data != 0);

    return 0;
}

int test_kmalloc_invalid() {
    void *data = kmalloc(HEAP_SIZE + 1);
    vassert(data == 0);

    return 0;
}

struct vsuite *memory_suite() {
    struct vsuite *suite = vsuite_create("Memory Tests", memory_reset);
    vsuite_add_test(suite, test_kmalloc_basic);
    vsuite_add_test(suite, test_kmalloc_full);
    vsuite_add_test(suite, test_kmalloc_invalid);
    return suite;
}
