#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/**
 * Perform a syscall with the following number.
 */
int syscall(unsigned int number);

void software_interrupt();

#endif
