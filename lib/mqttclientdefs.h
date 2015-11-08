#ifndef MQTTCLIENTDEFS_H
#define MQTTCLIENTDEFS_H


#define MAX_TOPICS 10
#define MAX_BUFFER_SIZE 66
// Recommended values for timers and counters. All timers are in milliseconds.
#define T_ADV 960000
#define N_ADV 3000
#define T_SEARCH_GW 5000
#define T_GW_INFO 5000
#define T_WAIT 360000
#define T_RETRY 15000
#define N_RETRY 5000
#define MQTT_DEBUG

#endif // MQTTCLIENTDEFS_H
