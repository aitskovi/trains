#ifndef _SYSCALL_H_
#define _SYSCALL_H_

struct Request;

enum syscall_number {
    CREATE,
    MY_TID,
    MY_PARENT_TID,
    PASS,
    EXIT,
    SEND,
    RECEIVE,
    REPLY,
    NUM_SYSCALLS,
};

/**
 * Perform a syscall with the following number.
 */
int syscall(struct Request *req);

int MyTid();
int MyParentTid();
int Create(int priority, void(*code)());
void Pass();
void Exit();

int Send(tid_t tid, char *msg, int msglen, char *reply, int replylen);
int Receive(tid_t *tid, char *msg, int msglen);
int Reply(tid_t tid, char *reply, int replylen);
#endif
