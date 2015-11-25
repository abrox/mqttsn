#ifndef UNIT_TESTS
#include "lib/mqttclientdefs.h"
#include <iostream>
#include "lib/udpnet.h"
#include "lib/mqttsnclient.h"
#include"lib/utils.h"
#include <unistd.h>
#include <list>
using namespace std;
typedef std::list<uint16_t> IdList;
typedef std::list<uint16_t>::iterator IdListIter;
IdList idList;

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

void connectStateHandler( MqttsnClient *client, const message_type mqttMsg);


UdpConfig udpConfig{"225.1.1.1",1884,2000};
MqttConfig mqttConfig{"Hihhuli",10,true,true,"jps/will","My will: please enjoy your life and have fun"};

//SerialNet serialNet;
UdpNet    net(udpConfig);
MqttsnClient mqttsn(net,mqttConfig,&connectStateHandler);
void publish();
ATimer pubT(&publish,false);


void handleTopicInfo(MqttsnClient *client,
                           const uint16_t topicId,
                           const message_type mqttMsg,
                           const uint8_t *msg,
                           uint8_t msgLen,
                           const uint16_t rCode)
{
    //cout << "handleTopicInfo"<< message_names[mqttMsg] <<" ID:" << topicId << " Rcode:" << rCode << endl;
   // cout << "handleTopicInfo: "<< mqttMsg<<" ID: " << topicId << "Rcode: " << rCode << endl;
    if (mqttMsg == REGACK){
     //  cout << " registered:" << endl;
       idList.push_back(topicId);

    }

}

void handleSeppo(MqttsnClient *client,
                           const uint16_t topicId,
                           const message_type mqttMsg,
                           const uint8_t *msg,
                           uint8_t msgLen,
                           const uint16_t rCode)
{
    //cout << "handleSeppo: "<< message_names[mqttMsg] <<" ID:" << topicId << " Rcode:" << rCode << endl;
    // cout << "handleTopicInfo: "<< mqttMsg<<" ID: " << topicId << "Rcode: " << rCode << endl;
    if( mqttMsg == PUBLISH ){
        for(int i =0; i < msgLen;i++)
            printf("%c",(char)msg[i]);
        printf("\n");


    }else
        if(mqttMsg== DISCONNECT){
            cout << "Seppo is disconnected\n";
        }

}

void handleTeppo(MqttsnClient *client,
                           const uint16_t topicId,
                           const message_type mqttMsg,
                           const uint8_t *msg,
                           uint8_t msgLen,
                           const uint16_t rCode)
{
   //cout << "handleTeppo: "<< message_names[mqttMsg] <<" ID:" << topicId << " Rcode:" << rCode << endl;
   // cout << "handleTopicInfo: "<< mqttMsg<<" ID: " << topicId << "Rcode: " << rCode << endl;
    if( mqttMsg == PUBLISH ){
        for(int i =0; i < msgLen;i++)
            printf("%c",(char)msg[i]);
        printf("\n");


    }else
     if(mqttMsg== DISCONNECT){
         cout << "Teppos is disconnected\n";
     }


}

void connectStateHandler( MqttsnClient *client,const message_type mqttMsg){
    if( mqttMsg == CONNACK){
        cout << "Hello from connected callback :-)"<< endl;
        client->register_topic(FLAG_QOS_0,"jps/mummo",&handleTopicInfo);
        client->register_topic(FLAG_QOS_0,"jps/pappa",&handleTopicInfo);
        client->register_topic(FLAG_QOS_0,"jps/poika",&handleTopicInfo);
        //client->register_topic(FLAG_QOS_0,"jps/hoitaja",&handleTopicInfo);

        client->subscribe_by_name(FLAG_QOS_0,"jps/teppo",&handleTeppo);
        client->subscribe_by_name(FLAG_QOS_0,"jps/seppo",&handleSeppo);
        pubT.start(2000);
    }
    else
    if( mqttMsg == DISCONNECT ){
        pubT.stop();
        idList.clear();
        cout << "Hello from handleDisconnected";
    }
}

const char*mesages[]{
    "Herra huu",
    "kukas muu",
    "anna vaa juu",
    "Hipuli ripuli",
    "Sulle mulle joo",
    "Osta skoda joo",
    "Annuli on ihana",
    "Joo Ja pikkasen lihava :-)",
    "Anna olla",
    "No en",
    "Outoa juttua ?",
    "Kuka on mukana ",
    "Sukka Make ja Alli huuppa",
    "No joo Aina se...",
    "Kuka Hitto täällä hilluu",
    "Smack krak ja pik pok",
    "HIH !"
};
const int messagesArraySize = sizeof(mesages)/sizeof(const char*);
uint8_t idx=0;
void publish(){

    for(IdListIter it= idList.begin();
        it != idList.end(); it++)
    {
        mqttsn.publish(*it,mesages[idx%messagesArraySize],strlen(mesages[idx%messagesArraySize]));
        idx++;
    }
}


int main()
{
    int rc;
    rc = mqttsn.initilize();

        while (!kbhit()) {
            mqttsn.run();
            pubT.run();
            usleep(10);
        }
    return rc;
}

#endif
