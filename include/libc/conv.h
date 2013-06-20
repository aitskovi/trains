#ifndef _CONV_H_
#define _CONV_H_

char c2x(char ch);
int a2d(char ch);
char a2i(char ch, char **src, int base, int *nump);
void ui2a(unsigned int num, unsigned int base, char *bf);
void i2a(int num, char *bf);

#endif
