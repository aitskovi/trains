#ifndef _SYSCALL_H_
#define _SYSCALL_H_

struct Request;

/**
 * Perform a syscall with the following number.
 */
int syscall(struct Request *req);

int MyTid(unsigned int specialNumber);

void software_interrupt();

#endif
