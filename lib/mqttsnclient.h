/*
mqttsnclient.h

The MIT License (MIT)

Copyright (C) 2014 John Donovan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef __MQTTSNCLIENT_H__
#define __MQTTSNCLIENT_H__

#include "mqttclientdefs.h"
#include "mqttsn.h"
#include "networkif.h"
#include "config.h"
#include "atimer.h"
#include"utils.h"

#ifdef ARDUINO
    #if ARDUINO >= 100
        #include "Arduino.h"
    #else
        #include "WProgram.h"
    #endif
#endif

#ifdef MQTT_DEBUG
#define CHANGESTATE(state)\
    _fsmStateName = #state;\
     D_PRINT("State:");\
     D_PRINTLN(_fsmStateName);\
    _fsmState = state;
#else
#define CHANGESTATE(state)\
    _fsmStateName = #state;\
    _fsmState = state;
#endif

class MqttsnClient;

typedef  void (*UserCBHdler)(MqttsnClient *client,const uint8_t *msg, uint8_t msgLen);

typedef  void (*TopicHdlr)(MqttsnClient *client,
                           const uint16_t &topicId,
                           const message_type &mqttMsg,
                           const uint8_t *msg,
                           uint8_t msgLen,
                           const uint16_t &rCode);


class MqttsnClient {
public:
    ///Construct MqqtClient
    ///
    MqttsnClient(NetworkIf &networkIf,///<NetIf. \note Thus it's reference Networkinterface can't be dekleted before this object !
                 const MqttConfig &mqttConfig
                 );
    virtual ~MqttsnClient();

    ///Initialize Client and network.
    /// In case network initilizing fails active timer and
    /// Retry after T_NETWORK_FAILED timeout.
    /// \return 0 in case success.
    ///
    int16_t initilize();

    ///Run Client.
    /// User must call this perioditically on loop.
    /// Handels message receiving & timeouts.
    ///
    int16_t run();


    bool registerUserMsgCallBack(message_type type, UserCBHdler cbFunct);

    bool publish(const uint16_t topic_id, const void* data, const uint8_t data_len);

    bool register_topic(const uint8_t flags,const char* name,TopicHdlr topicHdlr=NULL);

    bool subscribe_by_name(const uint8_t flags, const char* topic_name, TopicHdlr topicHdlr=NULL);

#ifndef UNIT_TESTS
protected:
#endif
    int16_t receiveMsg(uint8_t * buffer, const uint16_t buffSize, uint8_t &msgLen, message_type &msg);



    void searchgw(const uint8_t radius);
    void connect();
    void willtopic(const uint8_t flags, const char* will_topic, const bool update = false);
    void willmsg(const void* will_msg, const uint8_t will_msg_len, const bool update = false);
#ifdef USE_QOS2
    void pubrec();
    void pubrel();
    void pubcomp();
#endif
    void subscribe_by_id(const uint8_t flags, const uint16_t topic_id);
    void unsubscribe_by_name(const uint8_t flags, const char* topic_name);
    void unsubscribe_by_id(const uint8_t flags, const uint16_t topic_id);
    void pingreq(const char* client_id);
    void pingresp();
    void disconnect(const uint16_t duration);



    virtual void advertise_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void gwinfo_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void connack_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void willtopicreq_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void willmsgreq_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void regack_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void publish_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void register_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void puback_handler(const uint8_t *msg, uint8_t msgLen);
#ifdef USE_QOS2
    virtual void pubrec_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void pubrel_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void pubcomp_handler(const uint8_t *msg, uint8_t msgLen);
#endif
    virtual void suback_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void unsuback_handler(const uint8_t *msg, uint8_t msgLeng);
    virtual void pingreq_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void pingresp_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void disconnect_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void willtopicresp_handler(const uint8_t *msg, uint8_t msgLen);
    virtual void willmsgresp_handler(const uint8_t *msg, uint8_t msgLen);

    void regack(const uint16_t topic_id, const uint16_t message_id, const return_code_t return_code);
    void puback(const uint16_t topic_id, const uint16_t message_id, const return_code_t return_code);

#ifndef UNIT_TESTS
private:
#endif
    enum RegState{
        FREE,
        PUB_WAIT_SEND,
        PUB_WAIT_REG,
        PUB_REGISTERED,

        SUB_WAIT_SEND,
        SUB_WAIT_REG,
        SUB_REGISTERED
    };

    struct topic {
        const char* name;
        uint8_t     flags;
        uint16_t    id;
        TopicHdlr   hdlr;
        RegState    state;
        //Init corret values
        topic():name(NULL),flags(0),id(0),hdlr(NULL),state(FREE){;}
    };

    enum FSMState{
        INIT,
        NOT_CONNECTED,
        CONNECTING,
        CONNECTED,
        NETWORK_MISSING
    };
    enum rCode{
        REC_FAILED = -1,
        OK=0,
        NO_MESSAGE=1
    };

    void handleMsgIn(uint8_t msgLen, message_type msg);
    void send_message();
    topic* getTopicByState(RegState s);
    topic* getTopicById( uint16_t id );
    void handleGtwFound(uint8_t id);
    void handlePendingRegistrations();
    void handleSearchGTWTimeout();
    void handleNetMissingTimeout();
    void handleKeepAliveTimeout();

    uint16_t _message_id;

    uint8_t _gateway_id;

    NetworkIf &_networkIf;
    struct GtwInfo{
           uint8_t _id;
           NetworkAddr *_gtwAddr;
    };
    GtwInfo _gtwInfo;
    MqttConfig _mqttConfig;

    FSMState _fsmState;
    const char* _fsmStateName;
    uint8_t _message_buffer[MAX_BUFFER_SIZE];
    uint8_t _response_buffer[MAX_BUFFER_SIZE];
    topic topic_table[MAX_TOPICS];
    int16_t _pingCount;

    typedef  void (MqttsnClient::*MqttMsgHdler)(const uint8_t *msg, uint8_t msgLen);
    struct msgHdlr{
        const uint8_t _id;
        MqttMsgHdler _hdlr;
        UserCBHdler  _ucbhlr;
        //By introduse explisit constructor, without defaults,
        //it's inpossible to introduse handler array thats not initilized or is wrong size :-)
        msgHdlr(const uint8_t id,MqttMsgHdler hdlr):_id(id),_hdlr(hdlr),_ucbhlr(NULL){;}
    };
    #define HANDLER_ARRAY_SIZE 16
    msgHdlr _mqttMsgHdler[HANDLER_ARRAY_SIZE];

    enum timers{
        GTWSEARCH_TIMER,
        NET_MISSING_TIMER,
        KEEP_ALIVE_TIMER,
        TIMER_ARRAY_SIZE ///<Number of timers, must be last.
    };

    ATimer _timers[TIMER_ARRAY_SIZE];


};
#endif

#define SET_HANDLER(id,handler) {id,&MqttsnClient::handler}
#define MESSAGE_HANDLER_CALLBACS \
/*0x00 ADVERTISE*/    SET_HANDLER( ADVERTISE,    advertise_handler),\
/*0x02 GWINFO*/       SET_HANDLER( GWINFO,       gwinfo_handler),\
/*0x05 CONNACK*/      SET_HANDLER( CONNACK,      connack_handler),\
/*0x06 WILLTOPICREQ*/ SET_HANDLER( WILLTOPICREQ, willtopicreq_handler),\
/*0x08 WILLMSGREQ*/   SET_HANDLER( WILLMSGREQ,   willmsgreq_handler),\
/*0x0A REGISTER*/     SET_HANDLER( REGISTER,     register_handler),\
/*0x0B REGACK*/       SET_HANDLER( REGACK ,      regack_handler),\
/*0x0C PUBLISH*/      SET_HANDLER( PUBLISH,      publish_handler),\
/*0x0D PUBACK*/       SET_HANDLER( PUBACK ,      puback_handler),\
/*0x13 SUBACK*/       SET_HANDLER( SUBACK ,      suback_handler),\
/*0x15 UNSUBACK*/     SET_HANDLER( UNSUBACK,     unsuback_handler),\
/*0x16 PINGREQ*/      SET_HANDLER( PINGREQ ,     pingreq_handler),\
/*0x17 PINGRESP*/     SET_HANDLER( PINGRESP ,    pingresp_handler),\
/*0x18 DISCONNECT*/   SET_HANDLER( DISCONNECT,   disconnect_handler),\
/*0x1B WILLTOPICRESP*/SET_HANDLER( WILLTOPICRESP,willtopicresp_handler),\
/*0x1D WILLMSGRESP*/  SET_HANDLER( WILLMSGRESP,  willmsgresp_handler)

#define SET_TIMER(handler,isSingleShot){*this,&MqttsnClient::handler,isSingleShot}
#define CLIENT_TIMERS\
    SET_TIMER(handleSearchGTWTimeout,false),\
    SET_TIMER(handleNetMissingTimeout,false),\
    SET_TIMER(handleKeepAliveTimeout,false)
