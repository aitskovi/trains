#ifndef _WRITE_SERVER_H_
#define _WRITE_SERVER_H_

void WriteServer();

int Write(int channel, char *str, unsigned int size);
int Putc(int channel, char ch);

#endif
