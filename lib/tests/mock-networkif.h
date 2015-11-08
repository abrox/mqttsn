#ifdef UNIT_TESTS
#ifndef MOCNETWORKIF_H
#define MOCNETWORKIF_H

#include "gmock/gmock.h"
#include "../networkif.h"

class MockNetwork:public NetworkIf{
 public:
    MOCK_METHOD2(send, int(const uint8_t * buffer,uint16_t buffSize));
    MOCK_METHOD2(recv, int(uint8_t * buffer,uint16_t buffSize));
    MOCK_METHOD0(initilize, int16_t());
};

#endif // MOCNETWORKIF_H
#endif
