/*
 * Non-blocking IO rotines. These are converted
 * sends to the serial server.
 */


#include <sprintf.h>

#include <bwio.h>
#include <conv.h>
#include <syscall.h>
#include <write_server.h>
#include <read_server.h>
#include <string.h>

int sputw( char *buffer, int n, char fc, char *bf ) {
	char ch;
	char *p = bf;
	int size = 0;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) buffer[size++] = fc;
	while( ( ch = *bf++ ) ) buffer[size++] = ch;

	return size;
}

int sformat ( char *buffer, char *fmt, va_list va ) {
	char bf[12];
	char ch, lz;
	int w;
	int size = 0;
	int result;

	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			buffer[size++] = ch;
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
			case 0: return size;
			case 'c':
				buffer[size++] = va_arg( va, char );
				break;
			case 's':
				result = sputw( &buffer[size], w, 0, va_arg( va, char* ) );
				if (result < 0) {
				    return result;
				} else {
				    size += result;
				}
				break;
			case 'u':
				ui2a( va_arg( va, unsigned int ), 10, bf );
				result = sputw( &buffer[size], w, lz, bf );
                if (result < 0) {
                    return result;
                } else {
                    size += result;
                }
				break;
			case 'd':
				i2a( va_arg( va, int ), bf );
				result = sputw( &buffer[size], w, lz, bf );
                if (result < 0) {
                    return result;
                } else {
                    size += result;
                }
				break;
			case 'x':
				ui2a( va_arg( va, unsigned int ), 16, bf );
				result = sputw( &buffer[size], w, lz, bf );
                if (result < 0) {
                    return result;
                } else {
                    size += result;
                }
				break;
			case '%':
				buffer[size++] = ch;
				break;
			}
		}
	}

	buffer[size] = 0;
	return size;
}

int sprintf(char *buffer, char *fmt, ... ) {
    va_list va;
    int result;

    va_start(va,fmt);
    result = sformat( buffer, fmt, va );
    va_end(va);

    return result;
}
