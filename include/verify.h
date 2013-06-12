#ifndef _VERIFY_H_
#define _VERIFY_H_

#define MAX_NUM_TESTS 50

#define vassert(exp) if (!(exp)) {\
    log("%s failed at file: %s, line: %d\n", #exp, __FILE__, __LINE__); \
    return -1;\ 
}

#define vsuite_add_test(suite, test) __vsuite_add_test((suite), #test, test)

struct vtest {
    char *name;
    int(*test)();
};

int vtest_run();

struct vsuite {
    char *name;
    void(*setup)();
    struct vtest tests[50];
    int num_tests;
};

int vsuite_run(struct vsuite* suite);

#endif
