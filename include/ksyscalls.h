#ifndef _KSYSCALLS_H_
#define _KSYSCALLS_H_

struct Task;

int ksend(struct Task *active, int tid, char *msg, int msglen, char *reply, int replylen);
int krecieve(struct Task *active, int *tid, char *msg, int msglen);
int kreply(struct Task *active, int tid, char *reply, int replylen);
int kexit(struct Task *active);

#endif
