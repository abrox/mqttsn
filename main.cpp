#ifndef UNIT_TESTS
#include "lib/mqttclientdefs.h"
#include <iostream>
#include "lib/udpnet.h"
#include "lib/mqttsnclient.h"
#include"lib/utils.h"
#include <unistd.h>

using namespace std;

//TomyClient  -i ClientID  -g  225.1.1.1  -p 1883   -c  -t WillTopic  -m WillMessage -k 300
/*typedef struct {
    const char* ipAddress;
    uint16_t gPortNo;
}UdpConfig;*/

//typedef struct {
//    const char* nodeId;
//    uint16_t keepAlive;
//    bool     cleanSession;
//    bool     endDevice;
//    const char* willTopic;
//    const char* willMsg;
//}MqttsnConfig;

UdpConfig udpConfig{"225.1.1.1",1884,2000};
MqttConfig mqttConfig{"Hihhuli",10,true,true,"juu","WIL_MSG_ABC"};

//SerialNet serialNet;
UdpNet    net(udpConfig);
MqttsnClient mqttsn(net,mqttConfig);
void publish();
ATimer pubT(&publish,false);

void handleConnected(MqttsnClient *client,const uint8_t *msg, uint8_t msgLen){
    cout << "Hello from connected callback :-)"<< endl;
    client->register_topic("mummo");
}

uint16_t mummoID;
void handleTopicRegistered(MqttsnClient *client,const uint8_t *msg, uint8_t msgLen){
    cout << "Hello from handleTopicRegistered callback :-)"<< endl;
    uint8_t idx;
    mummoID = client->find_topic_id("mummo",idx);
    cout << "Mummo index: "<< static_cast<uint16_t>(idx)<< " and id: " <<mummoID << endl;
    client->subscribe_by_name(FLAG_QOS_0,"pappa");
    pubT.start(5000);
}
void handlePublishToMe(MqttsnClient *client,const uint8_t *msg, uint8_t msgLen){
   cout << "Hello from handlePublishToMe callback :-)"<< endl;
}

void publish(){
  mqttsn.publish(FLAG_QOS_1,mummoID,"voetokkiinsa",12);
}


int main()
{
    int rc;
    rc = mqttsn.initilize();
    if(!mqttsn.registerUserMsgCallBack(CONNACK,&handleConnected))
        cout << "Callbackregister failed?"<< endl;
    if(!mqttsn.registerUserMsgCallBack(REGACK,&handleTopicRegistered))
        cout << "Callbackregister failed?"<< endl;
    if(!mqttsn.registerUserMsgCallBack(PUBLISH,&handlePublishToMe))
        cout << "Callbackregister failed?"<< endl;

        while (!kbhit()) {
            mqttsn.run();
            pubT.run();
            usleep(10);
        }
    return rc;
}

#endif
