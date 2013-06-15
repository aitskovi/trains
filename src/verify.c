#include <verify.h>

#include <log.h>
#include <string.h>

#define HEAP_SIZE 1024 * 1024

static char vheap[HEAP_SIZE];
static char *vfree;

/**
 * Add a malloc for the testing framework. This is usually baadddd. But we want
 * to be able to run the testing framework on the arm cpu and we don't have malloc
 * there. Life's a lot easier with malloc, so we hack it together for now.
 */
void *vmalloc(unsigned int size) {
    if (vfree + size > vheap + HEAP_SIZE) {
        dlog("Heap unable to serve request for: %d\n", size);
        return 0;
    }

    void *ret = vfree;
    vfree += size;

    return ret;
}

int __vsuite_add_test(struct vsuite* suite, char *name, int(*test)()) {
    if (suite->num_tests >= MAX_NUM_TESTS) {
        log("Unable to add Test\n");
        return -1; 
    }

    // Construct test object.
    struct vtest* t = vmalloc(sizeof(struct vtest));
    t->name = vmalloc(strlen(name) + 1);
    strcpy(t->name, name);
    t->test = test;

    // Add the test object to test suite.
    suite->tests[suite->num_tests] = t;
    ++suite->num_tests;
    return 0;
}

int vtest_run(struct vtest* test) {
    log("Running %s\n", test->name);
    int result = test->test();
    if (result < 0) {
        log("Test %s failed\n", test->name);
    }
    return result;
}

struct vsuite* vsuite_create(char *name, void(*setup)()) {
    struct vsuite *suite = vmalloc(sizeof(struct vsuite));
    suite->name = vmalloc(strlen(name) + 1);
    strcpy(suite->name, name);
    suite->setup = setup;
    suite->num_tests = 0;
    return suite;
}

int vsuite_run(struct vsuite* suite) {
    log("Running Suite: %s\n", suite->name);

    int num_passed = 0;

    int i;
    for (i = 0; i < suite->num_tests; ++i) {
        struct vtest *test = suite->tests[i];
        if (suite->setup != 0) suite->setup();
        int result = vtest_run(test);
        if (result == 0) num_passed++;
    }
    
    log("Completed Suite: %s\n", suite->name);
    log("Tests Passed: %d, Failed: %d, Run: %d\n", num_passed, suite->num_tests - num_passed, suite->num_tests);

    return 0;
}

void verify_initialize() {
    vfree = vheap;
}
