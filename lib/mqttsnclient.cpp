/*
mqttsn-messages.cpp

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
#include "mqttclientdefs.h"


#include "mqttsnclient.h"
#include "mqttsn.h"

#include "utils.h"


MqttsnClient::MqttsnClient(NetworkIf &networkIf, const MqttConfig &mqttConfig) :
_message_id(0),
_gateway_id(0),
_networkIf(networkIf),
_gtwInfo{0xff,NULL},
_mqttConfig(mqttConfig),
_fsmState(INIT),
_fsmStateName(NULL),
_message_buffer{0},
_response_buffer{0},
_mqttMsgHdler{MESSAGE_HANDLER_CALLBACS},
_timers{CLIENT_TIMERS}
{

}

MqttsnClient::~MqttsnClient() {
}

int16_t MqttsnClient::initilize(){
    int16_t rc = _networkIf.initilize();

    if(!rc){
        searchgw(1);
        _timers[GTWSEARCH_TIMER].start(T_SEARCH_GW);
        CHANGESTATE(NOT_CONNECTED);
    }
    else{
        _timers[NET_MISSING_TIMER].start(T_NETWORK_FAILED);
        CHANGESTATE(NETWORK_MISSING);
    }

    return rc;
}

int16_t MqttsnClient::receiveMsg(uint8_t * buffer, const uint16_t buffSize, uint8_t &msgLen, message_type &msg){
    int16_t rc;
    rCode rc2;
    msgLen=0;

    rc = _networkIf.recv(buffer,buffSize);

    if( rc >= MSG_MIN_LEN ){
        msgLen = rc;
        msg=static_cast<message_type>(buffer[1]);
        rc2=OK;
        printOutMqttMsg(buffer,msgLen,true);
    }
    else
    if ( rc == 0 ){
        rc2 = NO_MESSAGE;
    }
    else {
       rc2 = REC_FAILED;
    }

    return rc2;
}
#define CALL_ME(object,ptrToMember) \
    ((object)->*(ptrToMember))

void MqttsnClient::handleMsgIn(uint8_t msgLen, message_type msg)
{
    for (uint8_t i=0;i<HANDLER_ARRAY_SIZE;i++) {
        if(_mqttMsgHdler[i]._id == msg){
            CALL_ME(this,_mqttMsgHdler[i]._hdlr) (_response_buffer,msgLen);
            if(_mqttMsgHdler[i]._ucbhlr)
                _mqttMsgHdler[i]._ucbhlr(this,_response_buffer,msgLen);
            break;
        }
    }
}

int16_t MqttsnClient::run(){
    uint8_t msgLen;
    message_type msg;
    int16_t rc;

    rc = receiveMsg(_response_buffer,MAX_BUFFER_SIZE,msgLen,msg);

    if(rc==OK) handleMsgIn(msgLen, msg);

    handlePendingRegistrations();

    for(uint8_t i=0;i< TIMER_ARRAY_SIZE;i++) _timers[i].run();

    return 0;
}

uint16_t MqttsnClient::find_topic_id(const char* name, uint8_t& index) {
    for (uint8_t i = 0; i < MAX_TOPICS; ++i) {
        if (strcmp(topic_table[i].name, name) == 0) {
            index = i;
            return topic_table[i].id;
        }
    }

    return 0xffff;
}
bool MqttsnClient::registerUserMsgCallBack(message_type type, UserCBHdler cbFunct){

    for (uint8_t i=0;i<HANDLER_ARRAY_SIZE;i++) {
        if(_mqttMsgHdler[i]._id == type){
           _mqttMsgHdler[i]._ucbhlr = cbFunct;
            return true;
        }
    }
  return false;
}

void MqttsnClient::send_message() {
    message_header* hdr = reinterpret_cast<message_header*>(_message_buffer);
    int16_t rc;

    printOutMqttMsg(_message_buffer,hdr->length,false);

    rc = _networkIf.send(_message_buffer, hdr->length,_gtwInfo._gtwAddr);
    if(rc < 0){
        // At this point lets start from begin, maybe in future
        //More desent way to recovery.
        _timers[NET_MISSING_TIMER].start(T_NETWORK_FAILED);
        CHANGESTATE(NETWORK_MISSING);
    }

}
///////////////////////////////////////////////////////////////////////////////////
////////////////////////////INCOMMING MESSAGE HANDELERS////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void MqttsnClient::handleGtwFound(uint8_t id)
{
    _gtwInfo._id = id;
    _gtwInfo._gtwAddr = _networkIf.getLastRecvAddr();
    _timers[GTWSEARCH_TIMER].stop();
    connect();
    CHANGESTATE(CONNECTING);
}

void MqttsnClient::advertise_handler(const uint8_t *m, uint8_t msgLen) {
    const msg_advertise *msg = reinterpret_cast<msg_advertise const *>(m);

    if(_fsmState == NOT_CONNECTED){
        handleGtwFound(msg->gw_id);
    }
}

void MqttsnClient::gwinfo_handler(const uint8_t *m, uint8_t msgLen) {
   const msg_gwinfo *msg= reinterpret_cast<msg_gwinfo const *>(m);

    if(_fsmState == NOT_CONNECTED){
        handleGtwFound(msg->gw_id);
    }
}

void MqttsnClient::connack_handler(const uint8_t *msg, uint8_t msgLen) {
    _timers[KEEP_ALIVE_TIMER].start(_mqttConfig.keepAlive*1000);
    CHANGESTATE(CONNECTED);
}

void MqttsnClient::willtopicreq_handler(const uint8_t *msg, uint8_t msgLen) {
    willtopic(FLAG_QOS_0,_mqttConfig.willTopic);
}

void MqttsnClient::willmsgreq_handler(const uint8_t *msg, uint8_t msgLen) {
    willmsg(_mqttConfig.willMsg,strlen(_mqttConfig.willMsg));
}



void MqttsnClient::regack_handler(const uint8_t *m, uint8_t msgLen) {
    const msg_regack *msg=reinterpret_cast<const msg_regack*>(m);
    //Only one should pending so this must find it
    topic*       t = getTopicByState(PUB_WAIT_REG);
    uint16_t msgId = bswap(msg->message_id);
    uint16_t    id = bswap(msg->topic_id);

    if(t->id != msgId){
        //Bad things has happened what to do ?
        //todo handle somehow
        return;
    }

    t->id = id;

    switch(msg->return_code){
       case ACCEPTED:
        t->state = PUB_REGISTERED;
        break;
    case REJECTED_CONGESTION:
        //If the return code was “rejected: congestion”, the client should wait for a time T W AIT before restarting the registration procedure
        break;
    default:

        break;

    }

    if(t->hdlr)
        t->hdlr(this,id,REGACK,reinterpret_cast<const uint8_t*>(t->name),strlen(t->name),msg->return_code);
}

void MqttsnClient::puback_handler(const uint8_t *msg, uint8_t msgLen) {
}

#ifdef USE_QOS2
void MQTTSN::pubrec_handler(const uint8_t *msg, uint8_t msgLen) {
}

void MQTTSN::pubrel_handler(const uint8_t *msg, uint8_t msgLen) {
}

void MQTTSN::pubcomp_handler(const uint8_t *msg, uint8_t msgLen) {
}
#endif

void MqttsnClient::pingreq_handler(const uint8_t *msg, uint8_t msgLen) {

    pingresp();
}

void MqttsnClient::suback_handler(const uint8_t *m, uint8_t msgLen) {

    const msg_suback *msg=reinterpret_cast<const msg_suback*>(m);
    //Only one should pending so this must find it
    topic*       t = getTopicByState(SUB_WAIT_REG);
    uint16_t msgId = bswap(msg->message_id);
    uint16_t    id = bswap(msg->topic_id);

    if(t->id != msgId){
        //Bad things has happened what to do ?
        //todo handle somehow
        return;
    }

    t->id = id;

    switch(msg->return_code){
       case ACCEPTED:
        t->state = SUB_REGISTERED;
        break;
    case REJECTED_CONGESTION:
        //If the return code was “rejected: congestion”, the client should wait for a time T W AIT before restarting the registration procedure
        break;
    default:

        break;

    }

    if(t->hdlr)
        t->hdlr(this,id,SUBACK,reinterpret_cast<const uint8_t*>(t->name),strlen(t->name),msg->return_code);
}

void MqttsnClient::unsuback_handler(const uint8_t *msg, uint8_t msgLen) {
}

void MqttsnClient::disconnect_handler(const uint8_t *msg, uint8_t msgLen) {
}

void MqttsnClient::pingresp_handler(const uint8_t *msg, uint8_t msgLen) {
}

void MqttsnClient::publish_handler(const uint8_t *m, uint8_t msgLen) {
    const msg_publish *msg=reinterpret_cast<const msg_publish*>(m);
    topic*            t=NULL;
    return_code_t     ret = REJECTED_INVALID_TOPIC_ID;
    const uint16_t    topic_id = bswap(msg->topic_id);

    for (uint8_t i = 0; i < MAX_TOPICS; ++i) {
        if (topic_table[i].id == topic_id) {
            t= &topic_table[i];
            ret = ACCEPTED;
            break;
        }
    }

    if (msg->flags & FLAG_QOS_1) {
        puback(topic_id, bswap(msg->message_id), ret);
    }

    if( t && t->hdlr)///\todo can message data be char or unsigned char in both places, yes i quess
        t->hdlr(this,topic_id,PUBLISH,reinterpret_cast<const uint8_t*>(&msg->data[0]),msgLen-sizeof(msg_publish),0);
}

//A GW sends a REGISTER message to a client if it wants to inform that client about the topic name and the
//assigned topic id that it will use later on when sending PUBLISH messages of the corresponding topic name.
//This happens for example when the client re-connects without having set the “CleanSession” flag or the client has
//subscribed to topic names that contain wildcard characters such as # or +.
///\todo WIldcard handling missing.
void MqttsnClient::register_handler(const uint8_t *m, uint8_t msgLen) {
    const msg_register *msg=reinterpret_cast<const msg_register *>(m);
    return_code_t ret = REJECTED_INVALID_TOPIC_ID;
    uint8_t index;
    find_topic_id(msg->topic_name, index);

    if (index != 0xff) {
        topic_table[index].id = bswap(msg->topic_id);
        ret = ACCEPTED;
    }

    regack(msg->topic_id, msg->message_id, ret);
}

void MqttsnClient::willtopicresp_handler(const uint8_t *msg, uint8_t msgLen) {
}

void MqttsnClient::willmsgresp_handler(const uint8_t *msg, uint8_t msgLen) {
}
///////////////////////////////////////////////////////////////////////////////////
////////////////////////////OUTGOING MQTT MESSAGES/////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void MqttsnClient::searchgw(const uint8_t radius) {
    msg_searchgw* msg = reinterpret_cast<msg_searchgw*>(_message_buffer);

    msg->length = sizeof(msg_searchgw);
    msg->type = SEARCHGW;
    msg->radius = radius;

    send_message();
}

void MqttsnClient::connect() {
    msg_connect* msg = reinterpret_cast<msg_connect*>(_message_buffer);
    uint8_t flags=0;
    flags|=_mqttConfig.cleanSession==true?FLAG_CLEAN:0;
    flags|=_mqttConfig.willTopic != NULL?FLAG_WILL:0;

    //onst uint8_t flags, const uint16_t duration, const char* client_id
    msg->length = sizeof(msg_connect) + strlen(_mqttConfig.nodeId);
    msg->type = CONNECT;
    msg->flags = flags;
    msg->protocol_id = PROTOCOL_ID;
    msg->duration = bswap(_mqttConfig.keepAlive);
    strcpy(msg->client_id, _mqttConfig.nodeId);

    send_message();
}

void MqttsnClient::willtopic(const uint8_t flags, const char* will_topic, const bool update) {
    if (will_topic == NULL) {
        message_header* msg = reinterpret_cast<message_header*>(_message_buffer);

        msg->type = update ? WILLTOPICUPD : WILLTOPIC;
        msg->length = sizeof(message_header);
    } else {
        msg_willtopic* msg = reinterpret_cast<msg_willtopic*>(_message_buffer);
        msg->length = sizeof(msg_willtopic) + strlen(will_topic);
        msg->type = update ? WILLTOPICUPD : WILLTOPIC;
        msg->flags = flags;
        strcpy(msg->will_topic, will_topic);
    }

    send_message();

//    if ((flags & QOS_MASK) == FLAG_QOS_1 || (flags & QOS_MASK) == FLAG_QOS_2) {
//        waiting_for_response = true;
//        response_to_wait_for = WILLMSGREQ;
//    }
}

void MqttsnClient::willmsg(const void* will_msg, const uint8_t will_msg_len, const bool update) {
    msg_willmsg* msg = reinterpret_cast<msg_willmsg*>(_message_buffer);

    msg->length = sizeof(msg_willmsg) + will_msg_len;
    msg->type = update ? WILLMSGUPD : WILLMSG;
    memcpy(msg->willmsg, will_msg, will_msg_len);

    send_message();
}

void MqttsnClient::disconnect(const uint16_t duration) {
    msg_disconnect* msg = reinterpret_cast<msg_disconnect*>(_message_buffer);

    msg->length = sizeof(message_header);
    msg->type = DISCONNECT;

    if (duration > 0) {
        msg->length += sizeof(msg_disconnect);
        msg->duration = bswap(duration);
    }

    send_message();
}

void MqttsnClient::handlePendingRegistrations(){
    topic * t;

    //Only one of the topics can wait registration at time
    if( !getTopicByState(PUB_WAIT_REG )){

        t=getTopicByState(PUB_WAIT_SEND);

        if(t){
            msg_register* msg = reinterpret_cast<msg_register*>(_message_buffer);

            ++_message_id;
            msg->length = sizeof(msg_register) + strlen(t->name);
            msg->type = REGISTER;
            msg->topic_id = 0;
            msg->message_id = bswap(_message_id);
            strcpy(msg->topic_name, t->name);

            t->id =_message_id;//Just to find this referense to this.
            t->state = PUB_WAIT_REG;

            send_message();
        }
    }
    //Only one of the topics can wait registration at time
    if( !getTopicByState(SUB_WAIT_REG )){

        t=getTopicByState(SUB_WAIT_SEND);

        if(t){
            msg_subscribe* msg = reinterpret_cast<msg_subscribe*>(_message_buffer);

            ++_message_id;
            // The -2 here is because we're unioning a 0-length member (topic_name)
            // with a uint16_t in the msg_subscribe struct.
            msg->length = sizeof(msg_subscribe) + strlen(t->name) - 2;
            msg->type = SUBSCRIBE;
            msg->flags = t->flags;
            msg->message_id = bswap(_message_id);
            msg->topic_id = 0;
            strcpy(msg->topic_name, t->name);

            t->id =_message_id;//Just to find this referense to this.
            t->state = SUB_WAIT_REG;

            send_message();
        }
    }
}

MqttsnClient::topic *MqttsnClient::getTopicByState(RegState s)
{
    for(uint8_t i=0; i < MAX_TOPICS;i++)
        if(topic_table[i].state == s)
            return &topic_table[i];

    return NULL;
}

bool MqttsnClient::register_topic(const char* name,TopicHdlr topicHdlr) {

    topic * t=getTopicByState(FREE);

    if(!t)
        return false;

    t->name  = name;
    t->id    = 0;
    t->hdlr  = topicHdlr;
    t->state = PUB_WAIT_SEND;

    return true;
}

void MqttsnClient::regack(const uint16_t topic_id, const uint16_t message_id, const return_code_t return_code) {
    msg_regack* msg = reinterpret_cast<msg_regack*>(_message_buffer);

    msg->length = sizeof(msg_regack);
    msg->type = REGACK;
    msg->topic_id = bswap(topic_id);
    msg->message_id = bswap(message_id);
    msg->return_code = return_code;

    send_message();
}

bool MqttsnClient::publish(const uint8_t flags, const uint16_t topic_id, const void* data, const uint8_t data_len) {

    if( data_len > (MAX_BUFFER_SIZE - sizeof(msg_publish)) || (!data && data_len>0))
        return false;

    msg_publish* msg = reinterpret_cast<msg_publish*>(_message_buffer);

    msg->length = sizeof(msg_publish) + data_len;
    msg->type = PUBLISH;
    msg->flags = flags;
    msg->topic_id = bswap(topic_id);
    msg->message_id = (flags & QOS_MASK == FLAG_QOS_0)?0:bswap(++_message_id);
    memcpy(msg->data, data, data_len);

    send_message();

    if ((flags & QOS_MASK) == FLAG_QOS_1 || (flags & QOS_MASK) == FLAG_QOS_2) {
        ;//todo handle waiting response !
    }
    return true;
}

#ifdef USE_QOS2
void MQTTSN::pubrec() {
    msg_pubqos2* msg = reinterpret_cast<msg_pubqos2*>(message_buffer);
    msg->length = sizeof(msg_pubqos2);
    msg->type = PUBREC;
    msg->message_id = bswap(_message_id);

    send_message();
}

void MQTTSN::pubrel() {
    msg_pubqos2* msg = reinterpret_cast<msg_pubqos2*>(message_buffer);
    msg->length = sizeof(msg_pubqos2);
    msg->type = PUBREL;
    msg->message_id = bswap(_message_id);

    send_message();
}

void MQTTSN::pubcomp() {
    msg_pubqos2* msg = reinterpret_cast<msg_pubqos2*>(message_buffer);
    msg->length = sizeof(msg_pubqos2);
    msg->type = PUBCOMP;
    msg->message_id = bswap(_message_id);

    send_message();
}
#endif

void MqttsnClient::puback(const uint16_t topic_id, const uint16_t message_id, const return_code_t return_code) {
    msg_puback* msg = reinterpret_cast<msg_puback*>(_message_buffer);

    msg->length = sizeof(msg_puback);
    msg->type = PUBACK;
    msg->topic_id = bswap(topic_id);
    msg->message_id = bswap(message_id);
    msg->return_code = return_code;

    send_message();
}

bool MqttsnClient::subscribe_by_name(const uint8_t flags, const char* topic_name,TopicHdlr topicHdlr) {
    if( !topic_name || strlen(topic_name) > MAX_BUFFER_SIZE - sizeof(msg_subscribe))
        return false;

    topic * t=getTopicByState(FREE);

    if(!t)
        return false;

    t->name  = topic_name;
    t->id    = 0;
    t->flags = (flags & QOS_MASK) | FLAG_TOPIC_NAME;
    t->hdlr  = topicHdlr;
    t->state = SUB_WAIT_SEND;

    return true;
}

void MqttsnClient::subscribe_by_id(const uint8_t flags, const uint16_t topic_id) {
    ++_message_id;

    msg_subscribe* msg = reinterpret_cast<msg_subscribe*>(_message_buffer);

    msg->length = sizeof(msg_subscribe);
    msg->type = SUBSCRIBE;
    msg->flags = (flags & QOS_MASK) | FLAG_TOPIC_PREDEFINED_ID;
    msg->message_id = bswap(_message_id);
    msg->topic_id = bswap(topic_id);

    send_message();

    if ((flags & QOS_MASK) == FLAG_QOS_1 || (flags & QOS_MASK) == FLAG_QOS_2) {
        ;//todo handle pending response
    }
}

void MqttsnClient::unsubscribe_by_name(const uint8_t flags, const char* topic_name) {
    ++_message_id;

    msg_unsubscribe* msg = reinterpret_cast<msg_unsubscribe*>(_message_buffer);

    // The -2 here is because we're unioning a 0-length member (topic_name)
    // with a uint16_t in the msg_unsubscribe struct.
    msg->length = sizeof(msg_unsubscribe) + strlen(topic_name) - 2;
    msg->type = UNSUBSCRIBE;
    msg->flags = (flags & QOS_MASK) | FLAG_TOPIC_NAME;
    msg->message_id = bswap(_message_id);
    strcpy(msg->topic_name, topic_name);

    send_message();

    if ((flags & QOS_MASK) == FLAG_QOS_1 || (flags & QOS_MASK) == FLAG_QOS_2) {
         ;//todo handle pending response
    }
}

void MqttsnClient::unsubscribe_by_id(const uint8_t flags, const uint16_t topic_id) {
    ++_message_id;

    msg_unsubscribe* msg = reinterpret_cast<msg_unsubscribe*>(_message_buffer);

    msg->length = sizeof(msg_unsubscribe);
    msg->type = UNSUBSCRIBE;
    msg->flags = (flags & QOS_MASK) | FLAG_TOPIC_PREDEFINED_ID;
    msg->message_id = bswap(_message_id);
    msg->topic_id = bswap(topic_id);

    send_message();

    if ((flags & QOS_MASK) == FLAG_QOS_1 || (flags & QOS_MASK) == FLAG_QOS_2) {
         ;//todo handle pending response
    }
}

void MqttsnClient::pingreq(const char* client_id) {
    msg_pingreq* msg = reinterpret_cast<msg_pingreq*>(_message_buffer);
    msg->length = sizeof(msg_pingreq) + strlen(client_id);
    msg->type = PINGREQ;
    strcpy(msg->client_id, client_id);

    send_message();

     ;//todo handle pending response
}

void MqttsnClient::pingresp() {
    message_header* msg = reinterpret_cast<message_header*>(_message_buffer);
    msg->length = sizeof(message_header);
    msg->type = PINGRESP;

    send_message();
}
//////////////////////////////////////////////////////////////////////////////
/////////////////////////////Timeout Handlers/////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void MqttsnClient::handleSearchGTWTimeout(){
    ///6.1Gateway Advertisement and Discovery\todo random delays.
    searchgw(1);
}
void MqttsnClient::handleNetMissingTimeout(){

    if( _gtwInfo._gtwAddr ){
        delete  _gtwInfo._gtwAddr;
         _gtwInfo._gtwAddr= NULL;
    }

    int16_t rc = initilize();
    if(!rc)
        _timers[NET_MISSING_TIMER].stop();
}
void MqttsnClient::handleKeepAliveTimeout(){
    pingreq(_mqttConfig.nodeId);
}
