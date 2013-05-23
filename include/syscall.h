#ifndef _SYSCALL_H_
#define _SYSCALL_H_

struct Request;

enum syscall_number {
    CREATE,
    MY_TID,
    MY_PARENT_TID,
    PASS,
    EXIT,
    NUM_SYSCALLS,
};

/**
 * Perform a syscall with the following number.
 */
int syscall(struct Request *req);

int MyTid(unsigned int specialNumber);
int MyParentTid();
int Create(int priority, void(*code)());
void Pass();
void Exit();

#endif
