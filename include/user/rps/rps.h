/*
 * rps.h
 *
 *  Created on: 2013-05-29
 *      Author: alex
 */

#ifndef RPS_H_
#define RPS_H_

enum RPSMessageType {
    SIGNUP,
    PLAY,
    QUIT,
    RESULT,
    GOAHEAD
};

enum RPSMoves {
    NONE,
    ROCK,
    PAPER,
    SCISSORS
};

enum RPSResults {
    WIN,
    LOSE,
    DRAW,
    FORFEIT
};

typedef struct RPSMessage {
    enum RPSMessageType type;
    enum RPSMoves move;
    enum RPSResults result;
} RPSMessage;


#endif /* RPS_H_ */
