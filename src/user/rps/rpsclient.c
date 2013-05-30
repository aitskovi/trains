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

#define GAMES_TO_PLAY 3

void rps_client () {
    tid_t rps_server = WhoIs("RPSServer");
    tid_t me = MyTid();

    RPSMessage msg, reply;

    msg.type = SIGNUP;
    Send(rps_server, &msg, sizeof(msg), &reply, sizeof(reply));

    // TODO assert(reply.type == GOAHEAD);

    for (int i = 0; i < GAMES_TO_PLAY; ++i) {
        msg.type = PLAY;
        // TODO pick a random move
        msg.move = PAPER;
        Send(rps_server, &msg, sizeof(msg), &reply, sizeof(reply));
        // TODO assert(reply.type == RESULT);
        if (FORFEIT == reply.result) {
            break;
        } else {
            if (WIN == reply.result) {
                bwprintf("I, task %u, won\n", me);
            } else if (LOSE == reply.result) {
                bwprintf("I, task %u, lost\n", me);
            }
        }
    }

    msg.type = QUIT;
    Send(rps_server, &msg, sizeof(msg), &reply, sizeof(reply));
    Exit();
}
