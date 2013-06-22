/*
 * Non-blocking IO rotines. These are converted
 * sends to the serial server.
 */


#include <nbio.h>

#include <bwio.h>
#include <conv.h>
#include <syscall.h>
#include <write_server.h>
#include <read_server.h>

int nbputc(int channel, char c) {
    return Putc(channel, c);
}

int nbgetc(int channel) {
    return Getc(channel);
}

int nbputx( int channel, char c ) {
	char chh, chl;

	chh = c2x( c / 16 );
	chl = c2x( c % 16 );
	nbputc( channel, chh );
	return nbputc( channel, chl );
}

int nbputr( int channel, unsigned int reg ) {
	int byte;
	char *ch = (char *) &reg;

	for( byte = 3; byte >= 0; byte-- ) nbputx( channel, ch[byte] );
	return nbputc( channel, ' ' );
}

int nbputstr( int channel, char *str ) {
	while( *str ) {
		if( nbputc( channel, *str ) < 0 ) return -1;
		str++;
	}
	return 0;
}

void nbputw( int channel, int n, char fc, char *bf ) {
	char ch;
	char *p = bf;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) nbputc( channel, fc );
	while( ( ch = *bf++ ) ) nbputc( channel, ch );
}

void nbformat ( int channel, char *fmt, va_list va ) {
	char bf[12];
	char ch, lz;
	int w;

	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			nbputc( channel, ch );
		else {
			lz = 0; w = 0;
			ch = *(fmt++);
			switch ( ch ) {
			case '0':
				lz = 1; ch = *(fmt++);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				ch = a2i( ch, &fmt, 10, &w );
				break;
			}
			switch( ch ) {
			case 0: return;
			case 'c':
				nbputc( channel, va_arg( va, char ) );
				break;
			case 's':
				nbputw( channel, w, 0, va_arg( va, char* ) );
				break;
			case 'u':
				ui2a( va_arg( va, unsigned int ), 10, bf );
				nbputw( channel, w, lz, bf );
				break;
			case 'd':
				i2a( va_arg( va, int ), bf );
				nbputw( channel, w, lz, bf );
				break;
			case 'x':
				ui2a( va_arg( va, unsigned int ), 16, bf );
				nbputw( channel, w, lz, bf );
				break;
			case '%':
				nbputc( channel, ch );
				break;
			}
		}
	}
}

void nbui2a( unsigned int num, unsigned int base, char *bf ) {
    int n = 0;
    int dgt;
    unsigned int d = 1;

    while( (num / d) >= base ) d *= base;
    while( d != 0 ) {
        dgt = num / d;
        num %= d;
        d /= base;
        if( n || dgt > 0 || d == 0 ) {
            *bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
            ++n;
        }
    }
    *bf = 0;
}

void nbprintf( int channel, char *fmt, ... ) {
        va_list va;

        va_start(va,fmt);
        nbformat( channel, fmt, va );
        va_end(va);
}
