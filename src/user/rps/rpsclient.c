/*
 * rpsclient.c
 *
 *  Created on: 2013-05-29
 *      Author: alex
 */

#include <syscall.h>
#include <nameserver.h>
#include <task.h>
#include <rpsclient.h>
#include <bwio.h>
#include <random.h>
#include <time.h>

#define GAMES_TO_PLAY 2

void rps_client () {
    tid_t me = MyTid();
    bwprintf(COM2, "RPS Client Task %d starting\n", me);

    Random random;
    seed_random(&random, me * 1234);

    tid_t rps_server = -1;
    while (rps_server < 0) {
        rps_server = WhoIs("RPSServer");
    }

    RPSMessage msg, reply;

    msg.type = SIGNUP;
    Send(rps_server, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    bwprintf(COM2, "RPS Client Task %d signed up\n", me);

    // TODO assert(reply.type == GOAHEAD);
    unsigned int i;
    for (i = 0; i < GAMES_TO_PLAY; ++i) {
        msg.type = PLAY;
        // TODO pick a random move
        msg.move = 1 + (rand_int(&random) % 3);
        Send(rps_server, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
        // TODO assert(reply.type == RESULT);
        if (OPPONENT_FORFEITED == reply.result) {
            break;
        } else {
            if (WIN == reply.result) {
                bwprintf(COM2, "I, task %u, won\n", me);
            } else if (LOSE == reply.result) {
                bwprintf(COM2, "I, task %u, lost\n", me);
            } else if (DRAW == reply.result) {
                bwprintf(COM2, "I, task %u, drew\n", me);
            }
        }
    }

//    msg.type = QUIT;
//    Send(rps_server, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));

    Exit();

}
