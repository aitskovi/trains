#ifndef _ENCODING_H_
#define _ENCODING_H_

#include <sensor_server.h>
#include <location_server.h>

enum MESSAGE_TYPE {
    SENSOR_SERVER_MESSAGE,
    LOCATION_SERVER_MESSAGE,
};

struct Message {
    enum MESSAGE_TYPE type;
    union {
        SensorServerMessage ss_msg;
        LocationServerMessage ls_msg;
    };
};

#endif
