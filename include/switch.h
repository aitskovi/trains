#ifndef _SWITCH_H_
#define _SWITCH_H_

struct Task;
struct Request;

unsigned int kernel_exit(struct Task *t);
void kernel_enter();

#endif
