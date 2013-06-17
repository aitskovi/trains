/*
 * test.c
 *
 *  Created on: May 22, 2013
 *      Author: aianus
 */

// TODO use a real unit testing library like check

#include <circular_queue.h>
#include <scheduling.h>
#include <task.h>
#include <verify.h>

void testTask () {
    return;
}

void scheduling_suite_reset() {
    initialize_tasks();
    initialize_scheduling();
}

int scheduling_operations_test() {
    Task *mediumTask = task_create(testTask, 0, MEDIUM);
    vassert (mediumTask->parent_tid == 0);
    vassert (task_get_pc(mediumTask) == testTask);
    vassert (mediumTask->priority == MEDIUM);
    vassert (mediumTask->tid == 0);

    Task *highTask = task_create(testTask, 0, HIGH);
    vassert (highTask->tid == 1);

    // Queues should be empty
    vassert (!schedule());

    // Make one ready
    make_ready(mediumTask);

    // Should be the next one up
    vassert (mediumTask == schedule());

    // Should now be empty again
    vassert (!schedule());

    // Now make both ready
    make_ready(mediumTask);
    make_ready(highTask);

    // Should be scheduled in priority order
    vassert (highTask == schedule());
    vassert (mediumTask == schedule());
    vassert (!schedule());

    // Medium should not run so long as high is ready
    make_ready(mediumTask);
    make_ready(highTask);

    vassert (highTask == schedule());
    make_ready(highTask);
    vassert (highTask == schedule());
    vassert (mediumTask == schedule());
    vassert (!schedule());

    return 0;
}

struct vsuite *scheduling_suite() {
    struct vsuite *suite = vsuite_create("Scheduling Tests", scheduling_suite_reset);
    vsuite_add_test(suite, scheduling_operations_test);
    return suite;
}
