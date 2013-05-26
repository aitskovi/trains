#ifndef _MESSAGING_H_
#define _MESSAGING_H_

/**
 * Send a message from a src to destination.
 *
 * @param src The source tid of the message.
 * @param dst The destination tid of the message.
 * @param msg The message to be sent.
 * @param msglen The length of the sent message.
 * @param reply A buffer for the reply to be put into.
 * @param replylen The length of the reply buffer.
 *
 * @return Result of the send comand. Can be either
 *       0 -> Successful Send
 *      -1 -> Error
 */
int ksend(int src, int dst, char *msg, int msglen, char *reply, int replylen);

/**
 * Recieve a message from another task.
 *
 * @param dst The tid recieving the message was sent to.
 * @param src The tid the message was sent from.
 * @param msg A buffer for holding the message.
 * @param msglen The length of the buffer for holding the message.
 *
 * @return Result of krecieve
 *      >0 -> Success getting message and length recieved.
 *      -1 -> SEND_BLOCKED
 *      -2 -> Error
 */
int krecieve(int dst, int *src, char *msg, int msglen);

/**
 * Replay to a message send by another task.
 *
 * @param tid The tid of the task you're replying to.
 * @param reply The contents of your reply.
 * @param replylen The length of your reply.
 *
 * @return Result of kreply
 *       >0  -> Success and Reply Length Sent.
 *       -1  -> Task is Recv_Blocked.
 *       -2  -> Task is Not Message Blocked.
 */
int kreply(int tid, char *reply, int replylen);

#endif
