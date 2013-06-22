#ifndef _NBIO_H_
#define _NBIO_H_

#define COM1 0
#define COM2 1

int nbputc(int channel, char c);
int nbgetc(int channel);
int nbputx(int channel, char c);
int nbputr(int channel, unsigned int reg);
int nbputstr(int channel, char *str);
void nbputw(int channel, int n, char fc, char *bf);
void nbprintf( int channel, char *fmt, ... );

#endif
