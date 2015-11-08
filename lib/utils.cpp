#include "mqttclientdefs.h"
#include "utils.h"


#ifdef MQTT_DEBUG
const char* message_names[] = {
    "ADVERTISE",
    "SEARCHGW",
    "GWINFO",
    "unknown",
    "CONNECT",
    "CONNACK",
    "WILLTOPICREQ",
    "WILLTOPIC",
    "WILLMSGREQ",
    "WILLMSG",
    "REGISTER",
    "REGACK",
    "PUBLISH",
    "PUBACK",
    "PUBCOMP",
    "PUBREC",
    "PUBREL",
    "unknown",
    "SUBSCRIBE",
    "SUBACK",
    "UNSUBSCRIBE",
    "UNSUBACK",
    "PINGREQ",
    "PINGRESP",
    "DISCONNECT",
    "unknown",
    "WILLTOPICUPD",
    "WILLTOPICRESP",
    "WILLMSGUPD",
    "WILLMSGRESP"
};
#endif

#ifdef LINUX
#include <iostream>
#include <cstdlib>
#include <sys/timeb.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


///Millisecond counter.
/// Keep
unsigned long millis(){
    timeb tb;
    ftime(&tb);
    unsigned long nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
    return nCount;
}

int getMilliSpan(int nTimeStart){
    int nSpan = millis() - nTimeStart;
    if(nSpan < 0)
        nSpan += 0x100000 * 1000;
    return nSpan;
}

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}


#endif //LINUX
