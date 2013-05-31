#ifndef _LOG_H_
#define _LOG_H_

/**
 * Log some data onto the screen.
 */
void log(char *format, ...);

/**
 * Log some debug data onto the screen. This only
 * gets printed if DEBUG is defined.
 */
void dlog(char *format, ...);

#endif
