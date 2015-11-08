#ifndef SERIALNET_H
#define SERIALNET_H

#include "mqttclientdefs.h"
#include "networkif.h"

class SerialNet : public NetworkIf
{
public:
    SerialNet();
    int send(const uint8_t * buffer,uint16_t buffSize){return 1;}
    int recv(uint8_t * buffer,uint16_t buffSize){return 0;}
    int16_t initilize();
};

#endif // SERIALNET_H
