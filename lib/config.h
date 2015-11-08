#ifndef CONFIG_H
#define CONFIG_H

#include "mqttclientdefs.h"

typedef struct {
    const char* nodeId;
    uint16_t keepAlive;
    bool     cleanSession;
    bool     endDevice;
    const char* willTopic;
    const char* willMsg;
}MqttConfig;



#endif // CONFIG_H
