#include <string.h>

/**
 * Return the length of a null terminated string.
 */
unsigned int strlen(char *a) {
    unsigned int length = 0;
    while (*a++) {
        ++length;
    }
    return length;
}

/**
 * Verify if two strings are equal.
 */
int streq(char *a, char *b) {
    while(*a != 0 && *b != 0 && *++a == *++b) {}
    return *a == *b;
}
