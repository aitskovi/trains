#ifndef _SERIAL_H_
#define _SERIAL_H_

enum WriteMessageType {
    WRITE_EVENT_REQUEST,
    WRITE_EVENT_RESPONSE,
    WRITE_CONFIG_REQUEST,
    WRITE_CONFIG_RESPONSE,
    WRITE_REQUEST,
    WRITE_RESPONSE
};

enum ReadMessageType {
    READ_EVENT_REQUEST,
    READ_EVENT_RESPONSE,
    READ_CONFIG_REQUEST,
    READ_CONFIG_RESPONSE,
    GETC_REQUEST,
    GETC_RESPONSE,
    CLEAR_REQUEST,
    CLEAR_RESPONSE,
};

typedef struct WriteMessage {
    enum WriteMessageType type;
    char * data;
    unsigned int length;
} WriteMessage;

typedef struct ReadMessage {
    enum ReadMessageType type;
    int data;
} ReadMessage;

/**
 * Send configuration information to a tid.
 */
void configure_writer(int tid, int channel);
void configure_reader(int tid, int channel);

/**
 * Recieve the configuration information.
 *
 * This blocks until config info arrives and returns
 * the channel which the task should operate on (COM1
 * or COM2). It also returns the tid the configuration
 * came from in the paramater.
 */
int get_reader_configuration(int *tid);
int get_writer_configuration(int *tid);

#endif
