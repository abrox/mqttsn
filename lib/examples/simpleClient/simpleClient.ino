#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <EEPROM.h>

#include"atimer.h"
#include "mqttclientdefs.h"
#include"utils.h"
#include "nrf24net.h"
#include "mqttsnclient.h"



MqttConfig mqttConfig{"Peppuli",10,true,true,"MyWIll_ABC","WIL_MSG_ABC"};

Nrf24Net    net;
MqttsnClient client(net,mqttConfig);
ATimer pubT(&publish,false);
uint16_t id;

void handleSeppo(MqttsnClient *client,
                           const uint16_t &topicId,
                           const message_type &mqttMsg,
                           const uint8_t *msg,
                           uint8_t msgLen,
                           const uint16_t &rCode)
{

    if( mqttMsg == PUBLISH ){
        ;
    }else
        if(mqttMsg== DISCONNECT){
            ;
        }

}

void handleMummo(MqttsnClient *client,
                           const uint16_t &topicId,
                           const message_type &mqttMsg,
                           const uint8_t *msg,
                           uint8_t msgLen,
                           const uint16_t &rCode)
{
    //cout << "handleTopicInfo"<< message_names[mqttMsg] <<" ID:" << topicId << " Rcode:" << rCode << endl;
   // cout << "handleTopicInfo: "<< mqttMsg<<" ID: " << topicId << "Rcode: " << rCode << endl;
    if (mqttMsg == REGACK){
     //  cout << " registered:" << endl;
      id = topicId;

    }
}
void handleConnected(MqttsnClient *client,const uint8_t *msg, uint8_t msgLen){
    client->register_topic(FLAG_QOS_0,"jps/mummo",&handleMummo);
    client->subscribe_by_name(FLAG_QOS_0,"jps/seppo",&handleSeppo);
    pubT.start(2000);
}

void handleDisconnected(MqttsnClient *client,const uint8_t *msg, uint8_t msgLen){
    pubT.stop();
}
void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting"));
  client.initilize();

  client.registerUserMsgCallBack(CONNACK,&handleConnected);
  client.registerUserMsgCallBack(DISCONNECT,&handleDisconnected);

  Serial.println(F("Initilized"));

}


void publish(){
   client.publish(id,"hei",3);
}

void loop() {
  client.run();
  pubT.run();
}
