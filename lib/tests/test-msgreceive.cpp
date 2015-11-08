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


TEST(ReceiveTest, NoMessage) {
    uint8_t buffer[MAX_BUFFER_SIZE]={0};
    uint8_t msgLen;
    message_type msg;
    MockNetwork mocNet;
    MqttsnClient mqttsn(mocNet,mqttConfig);
    EXPECT_CALL(mocNet, recv(_,MAX_BUFFER_SIZE))
          .Times(1)
          .WillOnce(Return(0));

   EXPECT_EQ(MqttsnClient::NO_MESSAGE,mqttsn.receiveMsg(buffer,MAX_BUFFER_SIZE,msgLen,msg));
}

TEST(ReceiveTest, TooShortMsg) {
    uint8_t buffer[MAX_BUFFER_SIZE]={0};
    uint8_t msgLen;
    message_type msg;
    MockNetwork mocNet;
    MqttsnClient mqttsn(mocNet,mqttConfig);
    EXPECT_CALL(mocNet, recv(_,MAX_BUFFER_SIZE))
          .Times(1)
          .WillOnce(Return(MSG_MIN_LEN -1));

   EXPECT_EQ(MqttsnClient::REC_FAILED,mqttsn.receiveMsg(buffer,MAX_BUFFER_SIZE,msgLen,msg));
}

TEST(ReceiveTest, RecvFailed) {
    uint8_t buffer[MAX_BUFFER_SIZE]={0};
    uint8_t msgLen;
    message_type msg;
    MockNetwork mocNet;
    MqttsnClient mqttsn(mocNet,mqttConfig);
    EXPECT_CALL(mocNet, recv(_,MAX_BUFFER_SIZE))
          .Times(2)
          .WillOnce(Return(-1))
          .WillOnce(Return(-2));

   EXPECT_EQ(MqttsnClient::REC_FAILED,mqttsn.receiveMsg(buffer,MAX_BUFFER_SIZE,msgLen,msg));
   EXPECT_EQ(MqttsnClient::REC_FAILED,mqttsn.receiveMsg(buffer,MAX_BUFFER_SIZE,msgLen,msg));
}
#endif

