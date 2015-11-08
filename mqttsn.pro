TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


SOURCES += main.cpp \
    lib/serialnet.cpp \
    lib/utils.cpp \
    lib/udpnet.cpp \
    lib/mqttsnclient.cpp \
    lib/tests/test-mqttclient.cpp \
    lib/nrf24net.cpp \
    lib/tests/test-msgreceive.cpp \
    ../atimer/atimer.cpp

HEADERS += \
    lib/mqttsn.h \
    lib/utils.h \
    lib/networkif.h \
    lib/serialnet.h \
    lib/udpnet.h \
    lib/config.h \
    lib/mqttsnclient.h \
    lib/tests/mock-networkif.h \
    lib/nrf24net.h \
    lib/mqttclientdefs.h \
     ../atimer/atimer.h
     ../atimer/callback.h

INCLUDEPATH += ../atimer/

#QMAKE_CXXFLAGS +=-DUNIT_TESTS
#LIBS += -lgtest_main
#LIBS += -lgmock_main
#LIBS += -lgtest
#LIBS += -lgmock
#LIBS += -lpthread

QMAKE_CXXFLAGS += -DLINUX
QMAKE_CXXFLAGS += -DMQTT_DEBUG
QMAKE_CXXFLAGS += -std=c++11
OTHER_FILES += \
    lib/examples/simpleClient/simpleClient.ino
