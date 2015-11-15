#ifndef UTILS_H
#define UTILS_H
#include "mqttclientdefs.h"

#ifdef LINUX
    #include <iostream>
    #include <cstdlib>
    #include <sys/timeb.h>
    #include <stdio.h>
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <iostream>
    #include <iomanip>
#else
   #if defined(ARDUINO) && ARDUINO >= 100
       #include "Arduino.h"
   #else
       #include "WProgram.h"
   #endif
#endif

#ifdef LINUX
///Millisecond counter.
/// Keep
unsigned long millis();
int getMilliSpan(int nTimeStart);
int kbhit(void);
#ifdef MQTT_DEBUG
extern const char* message_names[];
#endif
#endif

void printOutMqttMsg(const uint8_t * msg, uint8_t len, bool in );
uint16_t bswap(const uint16_t val);
/*======================================
      MACROs for debugging
========================================*/

#ifdef ARDUINO
    #ifdef MQTT_DEBUG
        #define D_PRINT(...)    Serial.print(__VA_ARGS__)
        #define D_PRINTLN(... ) Serial.println(__VA_ARGS__)
        #define D_PRINT_HEX(...)Serial.print(__VA_ARGS__,HEX)
    #else
        #define D_PRINT(...)
        #define D_PRINTLN(...)
        #define D_PRINT_HEX
    #endif
#endif
#ifdef LINUX
    #ifdef MQTT_DEBUG
        #define D_PRINT(...)   std::cout << __VA_ARGS__
        #define D_PRINTLN(...) std::cout << __VA_ARGS__ << std::endl
        #define D_PRINT_HEX(...) std::cout  << std::setw(4) << std::hex  <<__VA_ARGS__
        #define D_PRINTF(...) printf(__VA_ARGS__)
    #else
        #define D_PRINT(...)
        #define D_PRINTLN(...)
        #define D_PRINTF(...)
        #define D_PRINT_HEX(...)
    #endif
#endif

#endif// UTILS_H


