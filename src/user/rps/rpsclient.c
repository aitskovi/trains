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

#define GAMES_TO_PLAY 3

void rps_client () {
    tid_t me = MyTid();
    bwprintf(COM2, "RPS Client Task %d starting\n", me);

    tid_t rps_server = -1;
    while (rps_server < 0) {
        rps_server = WhoIs("RPSServer");
        bwprintf(COM2, "RPS Client Task %d got RPSServer tid as %d\n", me, rps_server);
    }

    /*

    RPSMessage msg, reply;

    msg.type = SIGNUP;
    Send(rps_server, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));

    // TODO assert(reply.type == GOAHEAD);
    unsigned int i;
    for (i = 0; i < GAMES_TO_PLAY; ++i) {
        msg.type = PLAY;
        // TODO pick a random move
        msg.move = PAPER;
        Send(rps_server, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
        // TODO assert(reply.type == RESULT);
        if (FORFEIT == reply.result) {
            break;
        } else {
            if (WIN == reply.result) {
                bwprintf(COM2, "I, task %u, won\n", me);
            } else if (LOSE == reply.result) {
                bwprintf(COM2, "I, task %u, lost\n", me);
            }
        }
    }

    msg.type = QUIT;
    Send(rps_server, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
*/
    Exit();

}
