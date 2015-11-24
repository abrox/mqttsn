#ifndef UDPNET_H
#define UDPNET_H

#include "mqttclientdefs.h"
#include "networkif.h"
#include "config.h"


#ifdef LINUX
#include <sys/time.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>


typedef struct {
    const char* ipAddress;
    uint16_t gPortNo;
    uint16_t uPortNo;
}UdpConfig;


#define STAT_UNICAST   1
#define STAT_MULTICAST 2

#define SOCKET_MAXHOSTNAME  200
#define SOCKET_MAXCONNECTIONS  5
#define SOCKET_MAXRECV  500
#define SOCKET_MAXBUFFER_LENGTH 500 // buffer size

#define PACKET_TIMEOUT_CHECK   200  // msec

class IpAddr:public NetworkAddr{
public:
    uint32_t _addr;
    uint16_t _port;
    IpAddr(uint32_t a=0,uint16_t p=0):_addr(a),_port(p){;}
};

class UdpNet : public NetworkIf
{
public:
    enum rCode{
        OK=0,
        INVALID_IP,
        INVALID_CONF,
        PORT_FAILED,
    };

    UdpNet(const UdpConfig &config);
    virtual ~UdpNet();
    int send(const uint8_t * buffer,const uint16_t buffSize,NetworkAddr *addr=NULL);
    int recv(uint8_t * buffer,uint16_t buffSize);
    int16_t initilize();
    NetworkAddr * getLastRecvAddr();

private:
    bool open( );
    int unicast(const uint8_t* buf, uint32_t length, uint32_t ipaddress, uint16_t port  );
    int multicast( const uint8_t* buf, uint32_t length );
    int recv(uint8_t* buf, uint16_t len, bool nonblock, uint32_t* ipaddress, uint16_t* port );
    int recv(uint8_t* buf, uint16_t len, int flags);
    bool checkRecvBuf();
    void close();
    int recvfrom ( uint8_t* buf, uint16_t len, int flags, uint32_t* ipaddress, uint16_t* port );

    UdpConfig _config;
    bool   _disconReq;
    int _sockfdUcast;
    int _sockfdMcast;
    uint8_t  _castStat;
    uint16_t _gPortNo;
    uint16_t _uPortNo;
    uint32_t _gIpAddr;
    IpAddr   _lastRecv;


};
#endif
#endif


