#ifndef NRF24NET_H
#define NRF24NET_H

#include "mqttclientdefs.h"
#include "networkif.h"

#ifdef ARDUINO
    #if ARDUINO >= 100
        #include "Arduino.h"
    #else
        #include "WProgram.h"
    #endif
#endif

class Nrf24Net : public NetworkIf
{
public:
    Nrf24Net();
    int send(const uint8_t * buffer,uint16_t buffSize,NetworkAddr *addr=NULL){return 1;}
    int recv(uint8_t * buffer,uint16_t buffSize){return 0;}
    int16_t initilize();
    NetworkAddr * getLastRecvAddr(){return NULL;}
};

#endif // NRF24NET_H
