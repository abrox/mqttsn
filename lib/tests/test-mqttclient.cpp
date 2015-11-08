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


TEST(ConnectTest, WillMessageRequired) {

}

TEST(ConnectTest, WillMessageNotRequired) {

}
TEST(ConnectTest, TestMessageSent) {
    MockNetwork mocNet;
    MqttsnClient mqttsn(mocNet,mqttConfig);
    EXPECT_CALL(mocNet, send(_,_))
          .Times(AtLeast(1));

   mqttsn.connect();

}
#endif
