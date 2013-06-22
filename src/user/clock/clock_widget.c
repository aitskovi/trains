/*
 * clock_widget.c
 *
 *  Created on: Jun 22, 2013
 *      Author: aianus
 */

#include <nbio.h>
#include <clock_server.h>

void clock_widget() {

    while (1) {
        int time = Time();
        unsigned int minutes, seconds, tenths;

        minutes = (time / 6000);
        time %= 6000;

        seconds = (time / 100);
        time %= 100;

        tenths = time / 10;

        // Save cursor, move to top left, clear line, print time, restore cursor
        nbprintf(COM2, "\0337\033[H\033[K%u:%u\.%u\0338", minutes, seconds, tenths);

        Delay(6);
    }

}
