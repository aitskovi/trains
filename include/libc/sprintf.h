#ifndef _SPRINTF_H_
#define _SPRINTF_H_

#include <bwio.h>

int sprintf( char *buffer, char *fmt, ... );
int sformat ( char *buffer, char *fmt, va_list va );
void ui2a( unsigned int num, unsigned int base, char *bf );
int sputw( char *buffer, int n, char fc, char *bf );

#endif
