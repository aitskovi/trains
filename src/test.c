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
#include <assert.h>

void testTask () {
    return;
}

int main () {

    initialize_tasks();
    initialize_scheduling();

    Task *mediumTask = task_create(testTask, 0, MEDIUM);
    assert (mediumTask->parent_tid == 0);
    assert (mediumTask->pc == testTask);
    assert (mediumTask->priority == MEDIUM);
    assert (mediumTask->tid == 0);

    Task *highTask = task_create(testTask, 0, HIGH);
    assert (highTask->tid == 1);

    // Queues should be empty
    assert (!schedule());

    // Make one ready
    make_ready(mediumTask);

    // Should be the next one up
    assert (mediumTask == schedule());

    // Should now be empty again
    assert (!schedule());

    // Now make both ready
    make_ready(mediumTask);
    make_ready(highTask);

    // Should be scheduled in priority order
    assert (highTask == schedule());
    assert (mediumTask == schedule());
    assert (!schedule());

    // Medium should not run so long as high is ready
    make_ready(mediumTask);
    make_ready(highTask);

    assert (highTask == schedule());
    make_ready(highTask);
    assert (highTask == schedule());
    assert (mediumTask == schedule());
    assert (!schedule());

}
