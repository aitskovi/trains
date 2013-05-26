void * memcpy (void *destination, void *source, unsigned int len) {
    char *dst = (char *)destination;
    char *src = (char *)source;

    unsigned int i;
    for (i = 0; i < len; ++i) {
        *dst++ = *src++;
    }

    return destination;
}
