#ifndef _SERIAL_H_
#define _SERIAL_H_

enum WriteMessageType {
    WRITE_EVENT_REQUEST,
    WRITE_EVENT_RESPONSE,
    WRITE_CONFIG_REQUEST,
    WRITE_CONFIG_RESPONSE,
    PUTC_REQUEST,
    PUTC_RESPONSE
};

enum ReadMessageType {
    READ_EVENT_REQUEST,
    READ_EVENT_RESPONSE,
    READ_CONFIG_REQUEST,
    READ_CONFIG_RESPONSE,
    GETC_REQUEST,
    GETC_RESPONSE
};

typedef struct WriteMessage {
    enum WriteMessageType type;
    int data;
} WriteMessage;

typedef struct ReadMessage {
    enum ReadMessageType type;
    int data;
} ReadMessage;

#endif
