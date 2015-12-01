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


void handleConnected( const message_type mqttMsg);
MqttConfig mqttConfig{"Peppuli",10,true,true,"jps/will","It all over"};

Nrf24Net    net;
MqttsnClient client(net,mqttConfig,&handleConnected);
ATimer pubT(&publish,false);
uint16_t id;

void handleSeppo(MqttsnClient *client,
                           const uint16_t topicId,
                           const message_type mqttMsg,
                           const uint8_t *msg,
                           uint8_t msgLen,
                           const uint16_t rCode)
{

    if( mqttMsg == PUBLISH ){
        Serial.write(msg,msgLen);
        Serial.println("");
    }
    else
    if( mqttMsg== DISCONNECT ){
        Serial.println(F("Disconnected handleSeppo"));
        pubT.stop();
    }

}

void handleMummo(MqttsnClient *client,
                           const uint16_t topicId,
                           const message_type mqttMsg,
                           const uint8_t *msg,
                           uint8_t msgLen,
                           const uint16_t rCode)
{
    if (mqttMsg == REGACK && rCode == ACCEPTED){
      id = topicId;
       Serial.println(F("handleMummo REGACK"));
    }
    else
    if( mqttMsg== DISCONNECT ){
        Serial.println(F("Disconnected handleMummo"));
        pubT.stop();
    }
}
void handleConnected( MqttsnClient *client, const message_type mqttMsg){
    if( mqttMsg == CONNACK){
        client->register_topic(FLAG_QOS_0,"jps/mummo",&handleMummo);
        client->subscribe_by_name(FLAG_QOS_0,"jps/seppo",&handleSeppo);
        client->subscribe_by_name(FLAG_QOS_0,"jps/pappa",&handleSeppo);
        pubT.start(1000);
    }
    else
    if( mqttMsg == DISCONNECT ){
        Serial.println(F("Disconnected handleConnected"));
        pubT.stop();
    }
}


void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting"));
  client.initilize();

  Serial.println(F("Initilized"));

}

uint16_t val=0;
void publish(){
   String s(val);
   val++;
   client.publish(id,s.c_str(),s.length());
}

void loop() {
  client.run();
  delay(10);
  pubT.run();
}
