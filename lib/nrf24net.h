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

#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"

class Nrf24Addr: public NetworkAddr
{

};

class Nrf24Net : public NetworkIf
{
public:
    Nrf24Net();
    int send(const uint8_t * buffer,const uint16_t buffSize,NetworkAddr *addr=NULL);
    int recv(uint8_t * buffer,uint16_t buffSize);
    int16_t initilize();
    NetworkAddr * getLastRecvAddr();

    void         setNodeId(uint8_t id){_mesh.setNodeID(id);}
    uint8_t      getNodeId(){return _mesh.getNodeID();}
private:
    RF24        _radio;
    RF24Network _network;
    RF24Mesh    _mesh;
};
#endif //Arduino
#endif // NRF24NET_H
