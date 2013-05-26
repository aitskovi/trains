#ifndef _MESSAGING_H_
#define _MESSAGING_H_

int ksend(int src_tid, int dst_tid, char *msg, int msglen, char *reply, int replylen);
int krecieve(int dst_tid, int *src_tid, char *msg, int msglen);
int kreply(int tid, char *reply, int replylen);

#endif
