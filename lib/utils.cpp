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
const char* RcodeNames[]= {
    "ACCEPTED",
    "REJECTED_CONGESTION",
    "REJECTED_INVALID_TOPIC_ID",
    "REJECTED_NOT_SUPPORTED"
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
    case CONNACK:
    {
        msg_connack const *m = reinterpret_cast< msg_connack const*>(msg);
        printf("rCode: %s(%d)\n",RcodeNames[bswap(m->return_code)%RCODE_MAX],bswap(m->return_code));
    }
        break;
    case WILLTOPIC:
    {
        msg_willtopic const *m = reinterpret_cast< msg_willtopic const*>(msg);
        printf("flags: %02x topic: ",m->flags);
        for(int i=0 ;i < (hdr->length-sizeof(msg_willtopic));i++ )
            printf("%c",m->will_topic[i]);
        printf("\n");
    }
        break;
    case WILLMSG:
    {
        msg_willmsg const *m = reinterpret_cast< msg_willmsg const*>(msg);
        printf("Will:");
        for(int i=0 ;i < (hdr->length-sizeof(msg_willmsg));i++ )
            printf("%c",m->willmsg[i]);
        printf("\n");
    }
        break;
    case REGISTER:
    {
        msg_register const *m = reinterpret_cast< msg_register const*>(msg);
        printf("topicId:%d messageID%d: ",bswap(m->topic_id),bswap(m->message_id));
        for(int i=0 ;i < (hdr->length-sizeof(msg_register));i++ )
            printf("%c",m->topic_name[i]);
        printf("\n");
    }
        break;
    case REGACK:
    case PUBACK://same data than regack
    {
        msg_regack const *m = reinterpret_cast< msg_regack const*>(msg);
        printf("topicId: %d messageID: %d rCode: %s(%d)\n",bswap(m->topic_id),
                                                           bswap(m->message_id),
                                                           RcodeNames[m->return_code%RCODE_MAX],
                                                           m->return_code);
    }
        break;
    case PUBLISH:
    {
        msg_publish const *m = reinterpret_cast< msg_publish const*>(msg);
        printf("flags: %02x topicId: %d messageID: %d",m->flags, bswap(m->topic_id),bswap(m->message_id));
        for(int i=0 ;i < (hdr->length-sizeof(msg_publish));i++ )
                  printf("%02x",m->data[i]);
        printf("\n");
    }
        break;
    case SUBSCRIBE:
    {
        msg_subscribe const *m = reinterpret_cast< msg_subscribe const*>(msg);
        printf("flags: %02x messageID: %d topic:",m->flags, bswap(m->message_id));
        for(int i=sizeof(msg_subscribe)-2 ;i < hdr->length;i++ )
                  printf("%c",msg[i]);
        printf("\n");
    }
        break;
    case SUBACK:
    {
        msg_suback const *m = reinterpret_cast< msg_suback const*>(msg);
        printf("flags:%02x topicId: %d messageID: %d rCode: %s(%d)\n",m->flags,
                                                        bswap(m->topic_id),
                                                        bswap(m->message_id),
                                                        RcodeNames[m->return_code%RCODE_MAX],
                                                        m->return_code);
    }
        break;

    case PINGREQ:
    {
        msg_pingreq const *m = reinterpret_cast< msg_pingreq const*>(msg);
        printf("\tClient:");
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
