#include "mqttclientdefs.h"
#include "utils.h"

#ifdef LINUX
#include <iostream>
#include <cstdlib>
#include <sys/timeb.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include"mqttsn.h"
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
void printOutMqttMsg(const uint8_t * msg, uint8_t len,bool in ){
    message_header const* hdr = reinterpret_cast<message_header const *>(msg);
    if( in )
        printf("IN: ");
    else
        printf("OUT:");

    if( len < MSG_MIN_LEN ){
        printf("Not mqtt msg\n");
        return;
    }

    printf("%d(%s)\t",hdr->type,message_names[hdr->type]);

    switch (hdr->type) {
    case ADVERTISE:
    {
        msg_advertise const *m = reinterpret_cast< msg_advertise const*>(msg);
        printf("GtwID: %d Duration: %d\n",m->gw_id,bswap(m->duration));
    }
        break;
    case SEARCHGW:
    {
        msg_searchgw const *m = reinterpret_cast< msg_searchgw const*>(msg);
        printf("Radius: %d\n",m->radius);
    }
        break;
    case GWINFO:
    {
        msg_gwinfo const *m = reinterpret_cast< msg_gwinfo const*>(msg);
        printf("GtwID: %d Addr:",m->gw_id);
        for(int i=0 ;i < (hdr->length-sizeof(msg_gwinfo));i++ )
            printf("%02x",m->gw_add[i]);
        printf("\n");
    }
        break;
    case CONNECT:
    {
        msg_connect const *m = reinterpret_cast< msg_connect const*>(msg);
        printf("flags: %02x proto :%02x, Duration: %d Client:",m->flags,m->protocol_id,bswap(m->duration));
        for(int i=0 ;i < (hdr->length-sizeof(msg_gwinfo));i++ )
            printf("%c",m->client_id[i]);
        printf("\n");
    }
        break;
    case PINGREQ:
    {
        msg_pingreq const *m = reinterpret_cast< msg_pingreq const*>(msg);
        printf("Client:");
        for(int i=0 ;i < (hdr->length-sizeof(msg_pingreq));i++ )
            printf("%c",m->client_id[i]);
        printf("\n");
    }
        break;
    default:
        printf("DATA:");
        for(int i=sizeof(message_header) ;i < hdr->length;i++ )
            printf("%02x",msg[i]);
        printf("\n");
        break;
    }

}
#endif

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
uint16_t bswap(const uint16_t val) {
    return (val << 8) | (val >> 8);
}
