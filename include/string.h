#ifndef _STRING_H_
#define _STRING_H_

/**
 * Return the length of a null terminated string.
 */
unsigned int strlen(char *a);

/**
 * Verify if two strings are equal.
 */
int streq(char *a, char *b);

/**
 * Copy a string to another string.
 */
char *strcpy(char *str, char *dst);

#endif
