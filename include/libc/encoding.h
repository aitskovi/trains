#ifndef _ENCODING_H_
#define _ENCODING_H_

#include <sensor_server.h>
#include <location_server.h>
#include <train_task.h>
#include <shell.h>
#include <switch_server.h>
#include <calibration.h>
#include <pubsub.h>
#include <reservation_server.h>

enum MESSAGE_TYPE {
    SENSOR_SERVER_MESSAGE,
    LOCATION_SERVER_MESSAGE,
    TRAIN_MESSAGE,
    SHELL_MESSAGE,
    SWITCH_SERVER_MESSAGE,
    CALIBRATION_MESSAGE,
    PUBSUB_MESSAGE,
    TRAIN_DISPLAY_MESSAGE,
    RESERVATION_SERVER_MESSAGE,
};

typedef struct Message {
    enum MESSAGE_TYPE type;
    union {
        SensorServerMessage ss_msg;
        LocationServerMessage ls_msg;
        TrainMessage tr_msg;
        ShellMessage sh_msg;
        SwitchServerMessage sw_msg;
        CalibrationMessage cs_msg;
        PubSubMessage ps_msg;
        ReservationServerMessage rs_msg;
    };
    int subscribers[MAX_SUBSCRIBERS];
} Message;

#endif
