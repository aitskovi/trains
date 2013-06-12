#include <log.h>
#include <verify.h>

int __vsuite_add_test(struct vsuite* suite, char *name, int(*test)()) {
    if (suite->num_tests >= MAX_NUM_TESTS) {
        log("Unable to add Test\n");
        return -1; 
    }

    struct vtest* t = &suite->tests[suite->num_tests];
    t->name = name;
    t->test = test;
    suite->num_tests++;
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

int vsuite_run(struct vsuite* suite) {
    log("Running Suite: %s\n", suite->name);

    int num_passed = 0;

    int i;
    for (i = 0; i < suite->num_tests; ++i) {
        struct vtest *test = &suite->tests[i];
        suite->setup();
        int result = vtest_run(test);
        if (result == 0) num_passed++;
    }
    
    log("Completed Suite: %s\n", suite->name);
    log("Tests Passed: %d, Failed: %d, Run: %d\n", num_passed, suite->num_tests - num_passed, suite->num_tests);

    return 0;
}
