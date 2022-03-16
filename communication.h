#ifndef COMMUNICATION
#define COMMUNICATION

#include "datastructures.h"

extern measurement m; //measurement to push received measurements
extern recv_profileT p; //profile to push received measurements

int getData(measurement* me, recv_profileT* pr);

/*Initializes blue tooth device with all configurations needed.*/
int init_blauzahn(int big_boss);

/*each step in arduino_loop for communicationthings
    recv data
    renew connection
    heartbeat
*/
int communication_loop(int big_boss, measurement* measure, recv_profileT* prof);

/* just send some data if it isn't something we can do better by our own methods
      Types
      O = String
			M = Measurement
			P = Profile
			B = bytes
      Sender
      M = Arduino Master
      S = Arduino Slave
      P = RaspberryPi
      returns
      -1 = failed
      -2 = to long buffer
      always starts with '_' so dont use this char!
*/
int send_header(char receiver, char type, int big_boss);
int send_header(char receiver, char type, char id, int big_boss);

/*Sends data and returns response.
 M = Arduino Master
 S = Arduino Slave
 P = RaspberryPi
*/
int send_measurements(char receiver, measurement *m, int big_boss);
int send_measurements(char receiver, measurement *m, unsigned char id, int big_boss);

#endif
