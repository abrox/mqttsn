#ifndef NETWORKIF_H
#define NETWORKIF_H

#include "mqttclientdefs.h"

#ifdef LINUX
    #include <stdint.h>
    #include<string.h>
#else
    #if defined(ARDUINO) && ARDUINO >= 100
       #include "Arduino.h"
    #else
       #include "WProgram.h"
    #endif
#endif

class NetworkAddr{
    public:
    NetworkAddr(){;}
    virtual ~NetworkAddr(){;}
};

///Public interface to network device.
///Very simple interface to the underlying network.
///Goal is that it changing network implementation.
///do not effect anyhow to actual MqttsnClient implementation at all.
///First state there will be implementatino for Linux UDP UdpNet,
///and NORDIC semiconductors nRF24L01 chip (Nrf24Net).
///Adding new implemnetations for eg. Zigbee should be easy.
///
class NetworkIf{
public:
    NetworkIf(){;}

    ///Send message to network.
    /// \return Number of bytes sent out.
    ///         In case of error negative number
    ///
    virtual int send(const uint8_t * buffer,
                     const uint16_t buffSize,
                     NetworkAddr *addr=NULL
                     )=0;

    virtual int recv(uint8_t * buffer,
                     uint16_t buffSize
                     )=0;
    ///Initilizing network connection.
    ///After initilizing it's possible to sen messages to the network
    /// What ever it means e.g opening serialport, create and open socket etc.
    /// Is resposible of the actual class taht implements interface.
    /// \returns Status of the operation.
    ///          \retval 0 in case success.
    ///          \retval !0 in case error, error codes specified by class that implements interface.
    ///
    virtual int16_t initilize()=0;

    /// Pointer to struct holds address of last received message sender.
    /// Caller must delete when ever do not need it anymore.
    ///
    virtual NetworkAddr * getLastRecvAddr()=0;
};


#endif // NETWORKIF_H
