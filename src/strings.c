#ifndef _STRINGS_H_
#define _STRINGS_H_

/**
 * Return the length of a null terminated string.
 */
int strlen(char *a) {
    // add 1 space for null terminator.
    int length = 1;
    while (*++a) {
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

#endif
