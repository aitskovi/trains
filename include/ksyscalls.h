#ifndef _KSYSCALLS_H_
#define _KSYSCALLS_H_

struct Task;

int ksend(struct Task *active, int tid, char *msg, int msglen, char *reply, int replylen);
int krecieve(struct Task *active, int *tid, char *msg, int msglen);
int kreply(struct Task *active, int tid, char *reply, int replylen);
int kexit(struct Task *active);
int kmytid(struct Task *active);
int kmy_parent_tid(struct Task *active);
int kcreate(struct Task *active, int priority, void(*code)(int), int arg);
int kwait_tid(struct Task *active, int tid);

#endif
