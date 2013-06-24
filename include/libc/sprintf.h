#ifndef _SPRINTF_H_
#define _SPRINTF_H_

int sprintf( char *buffer, char *fmt, ... );
void ui2a( unsigned int num, unsigned int base, char *bf );
int sputw( char *buffer, int n, char fc, char *bf );

#endif
