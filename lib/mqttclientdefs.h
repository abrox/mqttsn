#ifndef MQTTCLIENTDEFS_H
#define MQTTCLIENTDEFS_H


#define MAX_TOPICS 10
#define MAX_BUFFER_SIZE 66

//7.2 “Best practice” values for timers and counters
//Table 30 shows the “best practice” values for the timers and counters defined in this specification.
//Timer/Counter Recommended value
//T ADV         greater than 15 minutes
//N ADV         2 -3
//T SEARCHGW    5 seconds
//T GW INFO     5 seconds
//T W AIT       greater than 5 minutes
//T retry       10 - 15 seconds
//N retry       3-5

//Table 30: “Best practice” values for timers and counters
//The “tolerance” of the sleep and keep-alive timers at the server/gateway depends on the duration indicated by
//the clients. For example, the timer values should be 10% higher than the indicated values for durations larger than
//1 minute, and 50% higher if less.


// Recommended values for timers and counters. All timers are in milliseconds.
#define T_ADV 960000
#define N_ADV 3
#define T_SEARCH_GW 5000
#define T_GW_INFO 5000
#define T_WAIT 360000
#define T_RETRY 15000
#define N_RETRY 3

///Timeout for network failitures.
/// \todo What is good value for this ?
///       maybe it should increase like 5 sec 10 20 1 min...
#define T_NETWORK_FAILED 5000

#define MQTT_DEBUG

#endif // MQTTCLIENTDEFS_H
