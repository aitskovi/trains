/*
 * pathfinding_tests.c
 *
 *  Created on: Jul 4, 2013
 *      Author: aianus
 */

#include <verify.h>
#include <track_data.h>
#include <track_node.h>
#include <track.h>

int pathfinding_noreverse_test() {

    track_initialize('A');

    track_node *path[TRACK_MAX];
    unsigned int path_length;
    int result;

    track_node *C8, *BR3, *BR2, *BR1, *A12;

    vassert(C8 = track_get_by_name("C8"));
    vassert(BR3 = track_get_by_name("BR3"));
    vassert(BR2 = track_get_by_name("BR2"));
    vassert(BR1 = track_get_by_name("BR1"));
    vassert(A12 = track_get_by_name("A12"));

    // C8 to A12 pretty simple
    result = calculate_path(C8, A12, path, &path_length);

    // Should be able to calculate a path
    vassert (0 == result);

    vassert (C8 == path[0]);
    vassert (BR3 == path[1]);
    vassert (BR2 == path[2]);
    vassert (BR1 == path[3]);
    vassert (A12 == path[4]);

    return 0;

}

int pathfinding_simplereverse_test() {

    track_initialize('A');

    track_node *path[TRACK_MAX];
    unsigned int path_length;
    int result;

    track_node *C7, *C8, *BR3, *BR2, *BR1, *A12;

    vassert(C7 = track_get_by_name("C7"));
    vassert(C8 = track_get_by_name("C8"));
    vassert(BR3 = track_get_by_name("BR3"));
    vassert(BR2 = track_get_by_name("BR2"));
    vassert(BR1 = track_get_by_name("BR1"));
    vassert(A12 = track_get_by_name("A12"));

    // C8 to A12 pretty simple
    result = calculate_path(C7, A12, path, &path_length);

    // Should be able to calculate a path
    vassert (0 == result);

    vassert (C7 == path[0]);
    vassert (C8 == path[1]);
    vassert (BR3 == path[2]);
    vassert (BR2 == path[3]);
    vassert (BR1 == path[4]);
    vassert (A12 == path[5]);

    return 0;

}

void pathfinding_suite_reset() {

}

struct vsuite *pathfinding_suite() {
    struct vsuite *suite = vsuite_create("Pathfinding Tests", pathfinding_suite_reset);
    vsuite_add_test(suite, pathfinding_noreverse_test);
    vsuite_add_test(suite, pathfinding_simplereverse_test);
    return suite;
}
