/*
 * kernel2.c
 *
 *  Created on: 2013-05-29
 *      Author: alex
 */

#include <rpsclient.h>
#include <rpsserver.h>
#include <syscall.h>
#include <ts7200.h>

#define NUM_CLIENTS 6

void first() {

    /*
    // Spawn rps server
    Create(MEDIUM, rps_server);
    // Create two clients
    unsigned int i;
    for (i = 0; i < NUM_CLIENTS; ++i) {
        Create(MEDIUM, rps_client);
    }
    Exit();
    */

    // Cause a hardware interrupt to occur.
    int *irq_status = (int *)(VIC1_BASE + VIC_IRQ_STATUS_OFFSET);
    log("Interrupt Status: %x\n", *irq_status);
    log("Generating Interrupt\n");
    int *soft_int = (int *)(VIC1_BASE + VIC_SOFT_INT_OFFSET);
    *soft_int = 0x1;


    int i;
    for (i = 0; i < 3; ++i) {
        log("Generated Interrupt!\n");
        irq_status = (int *)(VIC1_BASE + VIC_IRQ_STATUS_OFFSET);
        log("Interrupt Status: %x\n", *irq_status);
    }

    Exit();
}
