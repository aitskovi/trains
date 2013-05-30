/*
 * timings.c
 *
 *  Created on: 2013-05-29
 *      Author: alex
 */

#include <bwio.h>
#include <syscall.h>
#include <time.h>

void first() {
    Timer timer;

    unsigned int i;
    for (i = 0; i < 4; ++i) {
        timer_reset(&timer);
        Pass();
        Time elapsed = timer_elapsed(&timer);
        bwprintf(COM2, "Round trip took %u usec\n", elapsed.useconds);
    }

    Exit();
}
