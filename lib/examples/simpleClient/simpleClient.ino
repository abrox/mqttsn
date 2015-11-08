
#include"atimer.h"
#include "mqttclientdefs.h"
#include"utils.h"
#include "nrf24net.h"
#include "mqttsnclient.h"



MqttConfig mqttConfig{"Peppuli",10,true,true,"MyWIll_ABC","WIL_MSG_ABC"};

Nrf24Net    net;
MqttsnClient mqttsn(net,mqttConfig);

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting"));
  mqttsn.initilize();
  Serial.println(F("Initilized"));

}



void loop() {
  mqttsn.run();
  delay(1000);
}
