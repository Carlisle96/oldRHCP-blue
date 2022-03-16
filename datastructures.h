#include <stdint.h>

#ifndef DATASTRUCTURES_H_
#define DATASTRUCTURES_H_

// struct used for the actual condition inside the box and also for the SensorDataReadout
typedef struct boxCondition {
	float temperature; 
	float humidity;
	unsigned int brightness; 
	unsigned int moisture;
	float outerTemperature;
} boxCondition;

// struct in which the profile is stored; temperature in *C, humidity in % and Duration in hours
typedef struct profile {
	float temperature;
	float humidity;
	float lightDuration;
} profile;

//1 = broken, 0 = working, indicates if sensor is working or not
typedef struct sensorStatus {
  bool tempHum;
  bool moisture;
  bool brightness;
} sensorStatus;

// values which indicate in which way the actors should be controlled
typedef struct actors {
        uint8_t light;          //activate led stripes seperately, every arduino has 2 stripes (2 bit, you're welcome Max!)
        bool fan;             
        bool heater;
} actors;

typedef struct controller { 
  uint8_t initMemory; 
  uint8_t sendAll; 
} controller;

// struct used for the actual condition inside the box and also for the SensorDataReadout
typedef struct measurement{
  int flag;
  float temperature;
  float humidity;
  unsigned int brightness;
  unsigned int moisture;
  unsigned long timestamp;
  float outer_temperature;
} measurement;

// struct in which the profile is stored; temperature in *C, humidity in % and Duration in hours
typedef struct recv_profileT {
  	int flag;
	float temperature;
	float humidity;
	int lightDuration;
	unsigned long timestamp; //timestamp of pi
} recv_profileT;

#endif
