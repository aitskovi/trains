#ifndef _SWITCH_H_
#define _SWITCH_H_

struct Task;

struct Request {
    unsigned int syscall;
    int result;
    void *args;
};

void kernel_exit(struct Task *t);
void kernel_enter(struct Task *t);

#endif
