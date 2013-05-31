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
    SCISSORS,
    FORFEIT
};

enum RPSResults {
    WIN,
    LOSE,
    DRAW,
    OPPONENT_FORFEITED
};

typedef struct RPSMessage {
    enum RPSMessageType type;
    enum RPSMoves move;
    enum RPSResults result;
} RPSMessage;


#endif /* RPS_H_ */
