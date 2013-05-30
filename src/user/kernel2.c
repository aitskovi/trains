/*
 * kernel2.c
 *
 *  Created on: 2013-05-29
 *      Author: alex
 */

#include <rpsclient.h>
#include <rpsserver.h>
#include <syscall.h>

#define NUM_CLIENTS 2

void first() {
    // Spawn rps server
    Create(MEDIUM, rps_server);
    // Create two clients
    unsigned int i;
    for (i = 0; i < NUM_CLIENTS; ++i) {
        Create(MEDIUM, rps_client);
    }
    Exit();
}
