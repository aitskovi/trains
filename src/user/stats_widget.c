#include <syscall.h>
#include <write_server.h>
#include <ts7200.h>
#include <encoding.h>
#include <dassert.h>

#define STATS_WIDGET_X 120 
#define STATS_WIDGET_Y 5 

#define STAT_COLUMN_WIDTH 10

#define COM1_STRING "COM1:"

static void stats_widget_init() {
    char buffer[100];
    char *pos = &buffer[0];

    pos += sprintf(pos, "\0337");

    pos += sprintf(pos, "\033[%u;%uH", STATS_WIDGET_Y, STATS_WIDGET_X);
    pos += sprintf(pos, "Stats:");

    pos += sprintf(pos, "\033[%u;%uH", STATS_WIDGET_Y + 1, STATS_WIDGET_X);
    pos += sprintf(pos, "%s 0", COM1_STRING);

    pos += sprintf(pos, "\0338");
    Write(COM2, buffer, pos - buffer);
}

static void stats_com1_update(int size) {
    char command[128];
    char *pos = &command[0];

    pos += sprintf(pos, "\0337");
    int offset = strlen(COM1_STRING);
    pos += sprintf(pos, "\033[%u;%uH", STATS_WIDGET_Y + 1, STATS_WIDGET_X + offset);
    char data[STAT_COLUMN_WIDTH];
    sprintf(data, "%d", size);
    pos += sputw(pos, STAT_COLUMN_WIDTH - offset, ' ', data);
    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
}

/**
 * Widget for displaying important stats.
 */
void stats_widget() {
    Subscribe("UART1WriteServerStream", PUBSUB_LOW);

    stats_widget_init();

    int tid;
    struct Message msg, rply;
    for(;;) {
        // Recieve a Write Message.
        Receive(&tid, (char *) &msg, sizeof(msg));
        cuassert(msg.type == WRITE_MESSAGE, "Invalid Message");

        // Ack Write Message.
        rply.type = WRITE_MESSAGE;
        Reply(tid, (char *) &rply, sizeof(rply));

        stats_com1_update(msg.ws_msg.size);
    }

    Exit();
}
