#ifdef UNIT_TESTS

#include "../mqttclientdefs.h"
#include "../mqttsnclient.h"
#include "mock-networkif.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace {
     MqttConfig mqttConfig{"Hihhuli",100,true,true,"MyWIll","Everything willl be fine"};
}

using ::testing::AtLeast;
using ::testing::_;
using ::testing::Return;


TEST(ReceiveTest, initilizeOK) {
    MockNetwork mocNet;
    MqttsnClient mqttsn(mocNet,mqttConfig);
    EXPECT_CALL(mocNet, initilize())
          .WillOnce(Return(0));
    EXPECT_CALL(mocNet, send(_,_))
          .WillOnce(Return(0));

   //Returns OK
   EXPECT_EQ(mqttsn.initilize(),0);
   //And change connecting state
   EXPECT_EQ(mqttsn._fsmState,MqttsnClient::FSMState::NOT_CONNECTED);
   //Activate search timer
   EXPECT_EQ(mqttsn._timers[MqttsnClient::timers::GTWSEARCH_TIMER]._enabled,true);
   //WIth correct time
   EXPECT_EQ(mqttsn._timers[MqttsnClient::timers::GTWSEARCH_TIMER]._msInterval,T_SEARCH_GW);
}


TEST(ReceiveTest, InitializeFails) {
    MockNetwork mocNet;
    MqttsnClient mqttsn(mocNet,mqttConfig);
    EXPECT_CALL(mocNet, initilize())
          .WillOnce(Return(-1));

   //Network init fails
   EXPECT_EQ(mqttsn.initilize(),-1);
   //And change Network missing state
   EXPECT_EQ(mqttsn._fsmState,MqttsnClient::FSMState::NETWORK_MISSING);
   //Activate NET_MISSING_TIMER timer
   EXPECT_EQ(mqttsn._timers[MqttsnClient::timers::NET_MISSING_TIMER]._enabled,true);
   //WIth correct time
   EXPECT_EQ(mqttsn._timers[MqttsnClient::timers::NET_MISSING_TIMER]._msInterval,T_NETWORK_FAILED);
}

#endif


