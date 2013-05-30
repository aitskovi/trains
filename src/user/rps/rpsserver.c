/*
 * rpsserver.c
 *
 *  Created on: 2013-05-29
 *      Author: alex
 */

#include <circular_queue.h>
#include <rps.h>
#include <syscall.h>
#include <task.h>
#include <memory.h>
#include <bwio.h>

typedef struct RPSMatch {
    tid_t task1;
    enum RPSMoves t1Move;
    tid_t task2;
    enum RPSMoves t2Move;
} RPSMatch;

RPSMatch * find_match_containing_tid(RPSMatch * matches, unsigned int size, tid_t tid) {
    unsigned int i;
    for (i = 0; i < size; ++i) {
        if (matches[i].task1 == tid || matches[i].task2 == tid) {
            return &matches[i];
        }
    }
    return 0;
}

RPSMatch * find_free_match_spot(RPSMatch * matches, unsigned int size) {
    unsigned int i;
    for (i = 0; i < size; ++i) {
        if (matches[i].task1 == 0 && matches[i].task2 == 0) {
            return &matches[i];
        }
    }
    return 0;
}

void rps_server () {

    tid_t me = MyTid();
    bwprintf(COM2, "RPS Server staring with tid %d\n", me);

    /*
    int result = -1;
    while (result < 0) {
        result = RegisterAs("RPSServer");
    }

    RPSMessage msg;
    tid_t tid;

    RPSMatch matches[MAX_TASKS/2 + 1];
    memset(matches, 0, sizeof(matches));
    struct circular_queue waiting_for_match;
    circular_queue_initialize(&waiting_for_match);


    RPSMatch *match;

    while (1) {
        bwprintf(COM2, "RPS Server waiting for message\n");

        // Receive a request and process it
        Receive(&tid, (char *) &msg, sizeof(msg));

        switch (msg.type) {
        case SIGNUP:
            bwprintf(COM2, "RPS Server got a signup\n");
            // If it's a signup
                // Put the task on the queue
                circular_queue_push(&waiting_for_match, (void *) tid);
                // If there are two tasks on the queue, pop them off to create a pair
                if (2 == circular_queue_size(&waiting_for_match)) {
                    RPSMatch *match = find_free_match_spot(matches, sizeof(matches)/sizeof(matches[0]));
                    match->task1 = (tid_t) circular_queue_pop(&waiting_for_match);
                    match->task2 = (tid_t) circular_queue_pop(&waiting_for_match);
                }
            break;
        case PLAY:
            bwprintf(COM2, "RPS Server got a play\n");
            // If it's a play
                // Check that the task is in a pair and we're expecting a play
                match = find_match_containing_tid(matches, sizeof(matches)/sizeof(matches[0]), tid);
                // Register the play
                if (tid == match->task1) {
                    match->t1Move = msg.move;
                } else {
                    match->t2Move = msg.move;
                }
                // If both tasks in the pair have played, reply with the result
                if (match->t1Move != NONE && match->t2Move != NONE) {
                    RPSMessage t1Reply;
                    RPSMessage t2Reply;

                    t1Reply.type = t2Reply.type = RESULT;

                    unsigned int winner = 0;
                    if (ROCK == match->t1Move) {
                        if (ROCK == match->t2Move) {
                            winner = 0;
                        }
                        else if (PAPER == match->t2Move) {
                            winner = 2;
                        } else if (SCISSORS == match->t2Move) {
                            winner = 1;
                        }
                    }
                    else if (PAPER == match->t1Move) {
                        if (ROCK == match->t2Move) {
                            winner = 1;
                        }
                        else if (PAPER == match->t2Move) {
                            winner = 0;
                        } else if (SCISSORS == match->t2Move) {
                            winner = 2;
                        }
                    }
                    else if (SCISSORS == match->t1Move) {
                        if (ROCK == match->t2Move) {
                            winner = 2;
                        }
                        else if (PAPER == match->t2Move) {
                            winner = 1;
                        } else if (SCISSORS == match->t2Move) {
                            winner = 0;
                        }
                    }

                    switch (winner) {
                    case 0:
                        t1Reply.result = t2Reply.result = DRAW;
                        break;
                    case 1:
                        t1Reply.result = WIN;
                        t2Reply.result = LOSE;
                        break;
                    case 2:
                        t1Reply.result = LOSE;
                        t2Reply.result = WIN;
                        break;
                    }

                    // TODO bwgetc here to pause?

                    Reply(match->task1, (char *) &t1Reply, sizeof(t1Reply));
                    Reply(match->task2, (char *) &t2Reply, sizeof(t2Reply));

                    match->t1Move = match->t2Move = NONE;
                }
            break;
        case QUIT:
            bwprintf(COM2, "RPS Server got a QUIT\n");
            // If it's a quit
                // Flag the pair for removal
                // Remove the quit task
            break;
        default:
            // Error or something
            break;
        }

    }
*/
    Exit();
}
