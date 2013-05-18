#ifndef _SWITCH_H_
#define _SWITCH_H_

struct Task;
struct Request;

void kernel_exit(struct Task *t, struct Request *req);
void kernel_enter();

#endif
