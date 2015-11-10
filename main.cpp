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



int main()
{
    int rc;
    rc = mqttsn.initilize();

        while (!kbhit()) {
            rc = mqttsn.run();
            if(rc){
                cout << "Run fail rc:"<< rc << endl;
            }
            usleep(10);
        }
    return rc;
}

#endif
