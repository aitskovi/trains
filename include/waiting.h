#ifndef _WAITING_H_
#define _WAITING_H_

#define NO_WAIT -1

struct Task;

void initialize_waiting();
int waiting_add(struct Task *task, struct Task *waiter);
struct Task *waiting_pop(struct Task *task);

#endif
