
#ifdef NET_DEBUG
    #define MQTT_DEBUG
#else
    #undef MQTT_DEBUG
#endif

#include "mqttclientdefs.h"
#include "udpnet.h"

#include "utils.h"

#ifdef LINUX



#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>




UdpNet::UdpNet(const UdpConfig &config):
    _config(config),
    _disconReq(false),
    _sockfdUcast(-1),
    _sockfdMcast(-1),
    _castStat(0),
    _gPortNo(0),
    _uPortNo(0),
    _gIpAddr(0)
{
}

UdpNet::~UdpNet(){
    close();
}

int16_t UdpNet::initilize(){
    struct in_addr address;

    if(!inet_aton(_config.ipAddress,&address))
        return INVALID_IP;

    _gPortNo = htons(_config.gPortNo);
    _gIpAddr = address.s_addr;
    _uPortNo = htons(_config.uPortNo);

    _lastRecv._addr=0;
    _lastRecv._port=0;

    if( _gPortNo == 0 || _gIpAddr == 0 || _uPortNo == 0)
        return INVALID_CONF;
    else if (!open())
        return PORT_FAILED;

    return OK;
}
 NetworkAddr * UdpNet::getLastRecvAddr(){
      NetworkAddr *a= new  NetworkAddr;
      *a=_lastRecv;
     return a;
 }

void UdpNet::close(){
    if(_sockfdMcast > 0){
        ::close( _sockfdMcast);
        _sockfdMcast = -1;
    if(_sockfdUcast > 0){
            ::close( _sockfdUcast);
            _sockfdUcast = -1;
        }
    }
}
int UdpNet::send(const uint8_t * buffer,uint16_t buffSize, NetworkAddr *addr){
    int rc;

    if(addr)
        rc = unicast(buffer,buffSize,_lastRecv._addr,_gPortNo);
    else
        rc = multicast(buffer,buffSize);
    return rc;
}
int UdpNet::recv(uint8_t * buffer,uint16_t buffSize){
    int stat=0;

    if(checkRecvBuf()){
        stat=recv(buffer,buffSize,true,&_lastRecv._addr,&_lastRecv._port);

    }
    return stat;
}
bool UdpNet::open(){
    const int reuse = 1;
    char loopch = 0;


    _sockfdUcast = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sockfdUcast < 0){
        return false;
    }

    setsockopt(_sockfdUcast, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = _uPortNo;
    addr.sin_addr.s_addr = INADDR_ANY;

    if( ::bind ( _sockfdUcast, (struct sockaddr*)&addr,  sizeof(addr)) <0){
        return false;
    }

    _sockfdMcast = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_sockfdMcast < 0){
        return false;
    }

    struct sockaddr_in addrm;
    addrm.sin_family = AF_INET;
    addrm.sin_port = _gPortNo;
    addrm.sin_addr.s_addr = INADDR_ANY;

    setsockopt(_sockfdMcast, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    if( ::bind ( _sockfdMcast, (struct sockaddr*)&addrm,  sizeof(addrm)) <0){
        return false;
    }

    if(setsockopt(_sockfdUcast, IPPROTO_IP, IP_MULTICAST_LOOP,(char*)&loopch, sizeof(loopch)) <0 ){
        D_PRINTF("error IP_MULTICAST_LOOP in UdpNet::open\n");

        close();
        return false;
    }

    if(setsockopt(_sockfdMcast, IPPROTO_IP, IP_MULTICAST_LOOP,(char*)&loopch, sizeof(loopch)) <0 ){
        D_PRINTF("error IP_MULTICAST_LOOP in UdpPPort::open\n");
        close();
        return false;
    }

    ip_mreq mreq;
    mreq.imr_interface.s_addr = INADDR_ANY;
    mreq.imr_multiaddr.s_addr = _gIpAddr;

    if( setsockopt(_sockfdMcast, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq) )< 0){
        D_PRINTF("error IP_ADD_MEMBERSHIP in UdpNet::open\n");
        close();
        return false;
    }
/*
    if( setsockopt(_sockfdUcast, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq) )< 0){
        D_PRINTF("error IP_ADD_MEMBERSHIP in UdpNet::open\n");
        close();
        return false;
    }
*/
    return true;
}

int UdpNet::unicast(const uint8_t* buf, uint32_t length, uint32_t ipAddress, uint16_t port  ){
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = port;
    dest.sin_addr.s_addr = ipAddress;

    int status = ::sendto( _sockfdUcast, buf, length, 0, (const sockaddr*)&dest, sizeof(dest) );
    if( status < 0){
        D_PRINTF("errno == %d in UdpNet::unicast\n", errno);
    }else{
        D_PRINTF("sendto %s:%u  [",inet_ntoa(dest.sin_addr),htons(port));
        for(uint16_t i = 0; i < length ; i++){
            D_PRINTF(" %02x", *(buf + i));
        }
        D_PRINTF(" ]\n");
    }
    return status;
}


int UdpNet::multicast( const uint8_t* buf, uint32_t length ){
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = _gPortNo;
    dest.sin_addr.s_addr = _gIpAddr;

    int status = ::sendto( _sockfdMcast, buf, length, 0, (const sockaddr*)&dest, sizeof(dest) );
    if( status < 0){
        D_PRINTF("errno == %d in UdpNet::multicast\n", errno);
    }else{
        D_PRINTF("sendto %s:%u  [",inet_ntoa(dest.sin_addr),htons(_gPortNo));
        for(uint16_t i = 0; i < length ; i++){
            D_PRINTF(" %02x", *(buf + i));
        }
        D_PRINTF(" ]\n");
    }
    return errno;
}

bool UdpNet::checkRecvBuf(){
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000;    // 500 msec

    uint8_t buf[2];
    fd_set recvfds;
    int maxSock = 0;

    FD_ZERO(&recvfds);
    FD_SET(_sockfdUcast, &recvfds);
    FD_SET(_sockfdMcast, &recvfds);

    if(_sockfdMcast > _sockfdUcast){
        maxSock = _sockfdMcast;
    }else{
        maxSock = _sockfdUcast;
    }

    select(maxSock + 1, &recvfds, 0, 0, &timeout);

    if(FD_ISSET(_sockfdUcast, &recvfds)){
        if( ::recv(_sockfdUcast, buf, 1,  MSG_DONTWAIT | MSG_PEEK) > 0){
            _castStat = STAT_UNICAST;
            return true;
        }
    }else if(FD_ISSET(_sockfdMcast, &recvfds)){
        if( ::recv(_sockfdMcast, buf, 1,  MSG_DONTWAIT | MSG_PEEK) > 0){
            _castStat = STAT_MULTICAST;
            return true;
        }
    }
    _castStat = 0;
    return false;
}

int UdpNet::recv(uint8_t* buf, uint16_t len, bool flg, uint32_t* ipAddressPtr, uint16_t* portPtr){
    int flags = flg ? MSG_DONTWAIT : 0;
    return recvfrom (buf, len, flags, ipAddressPtr, portPtr );
}

int UdpNet::recvfrom ( uint8_t* buf, uint16_t len, int flags, uint32_t* ipAddressPtr, uint16_t* portPtr ){
    struct sockaddr_in sender;
    int status;
    socklen_t addrlen = sizeof(sender);
    memset(&sender, 0, addrlen);

    if(_castStat == STAT_UNICAST){
        status = ::recvfrom( _sockfdUcast, buf, len, flags, (struct sockaddr*)&sender, &addrlen );
    }else if(_castStat == STAT_MULTICAST){
        status = ::recvfrom( _sockfdMcast, buf, len, flags, (struct sockaddr*)&sender, &addrlen );
    }else{
        return 0;
    }

    if (status < 0 && errno != EAGAIN)	{
        D_PRINTF("errno == %d in UdpNet::recvfrom \n", errno);
    }else if(status > 0){
        *ipAddressPtr = sender.sin_addr.s_addr;
        *portPtr = sender.sin_port;
        D_PRINTF("recved from %s:%u [",inet_ntoa(sender.sin_addr), htons(*portPtr));
        for(uint16_t i = 0; i < status ; i++){
            D_PRINTF(" %02x", *(buf + i));
        }
        D_PRINTF(" ]\n");
    }else{
        return 0;
    }
    return status;
}


#endif

