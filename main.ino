#include "avr/wdt.h"
#include <stdint.h>
#include "datastructures.h"
#include "functions.h"
#include "communication.h"
#include "Time.h"
#include "RGBdriver.h"
#include <Wire.h>
#include <Digital_Light_TSL2561.h>
// 1 if the arduino is the bigboss - 0 if nappel
#define RATE_OF_SAVES 900
#define RATE_OF_SENDS 30
#define MAX_SETS_OF_DATA 203
#define ARDUINO 1

// Contains the information of the sensors currently connected to the actual arduino
boxCondition rawSensorData;
#if ARDUINO == 1
	// Variables only for Bigboss
	boxCondition realCondition; 
	sensorStatus statusOfSensorsNappel;
	// Data of Nappel Sensors
	boxCondition nappelSensorData;
	// Actual Condition of the box which the actors are controlled by
#else
	// Variables only for Nappel
	#define realCondition rawSensorData
	uint8_t isMaster = 0;
#endif

// structs ONLY used for temporary storage and send
measurement m;
recv_profileT p;

// contains current profile values
profile currentProfile;

// contains values for actor controling
actors actorsCommands;

//contains status of sensors
sensorStatus statusOfSensors;

// This is the current time of the machine inside its current loop
uint32_t timestamp;
uint32_t lastTime = 0;// for sends
uint32_t lastTime2 = 0;// for saves

// a buffer to store the current actions of the loop
controller toDo;

// stores the received Flag from Raspi
int raspiFlag;

// counts the number of consecutive loops in which no commands have been received from Bigboss 
int heartbeatCounter = -1;
// counter for sendMeasurements
uint8_t sendCounter = 0;
uint8_t sendCounterP;
// counter for sendEverything
uint8_t dataSet = 0;

void setup() {

    pinMode(5, OUTPUT); // fan
    pinMode(6, OUTPUT); // heater

    Wire.begin();
	Serial.begin(115200);

	// initMemory();
	// TODO ( temp testing )
	// toDo.initMemory = 1;
	init_blauzahn(ARDUINO);

#if ARDUINO == 1
  	TSL2561.init();
#endif

  	// temporary timefix
  	setTime(1467262770);
    wdt_enable(WDTO_8S);

    raspiFlag = 0;
    heartbeatCounter = 0;
    sendCounter = 0;
    sendCounterP = 0;
    dataSet = 0;
    lastTime = 0;

    Serial.print(F("[OK] :: init completed \n"));

}

void loop() {

	/* - - - 
	INPUT
	- - - */

	// do we need a 'reset temp-communication-structs' function ? 
	m.flag = 0;
	m.temperature = 0;
	m.humidity = 0;
	m.brightness = 0;
	m.moisture = 0;
	m.timestamp = 0;
	m.outer_temperature = 0;
	p.flag = 0;
	p.temperature = 0; 
	p.humidity = 0; 
	p.lightDuration = 0; 
	p.timestamp = 0; 

	// recieves all recievable objects ( m, p ) ( takes about 12 secounds if no connection is setup yet )
	communication_loop(ARDUINO, &m, &p);
	wdt_reset();
  if(m.flag!=0 || p.flag!=0){
    Serial.print(F("<:: === === === === === :: === === === === === :: === === MEA === === :: === === === === === :: === === === === === ::>\n"));
  	Serial.print(F("<:: === === === === === :: === === === === === :: "));
  	Serial.print("\t");
  	Serial.print(m.flag);
  	Serial.print("\t");
  	Serial.print(m.temperature); 
  	Serial.print("\t");
  	Serial.print(m.humidity); 
  	Serial.print("\t");
  	Serial.print(m.brightness); 
  	Serial.print("\t");
  	Serial.print(m.moisture);
  	Serial.print("\t"); 
  	Serial.print(m.timestamp);
  	Serial.print("\t"); 
  	Serial.print(m.outer_temperature);
  	Serial.print("\t"); 
  	Serial.print(F(" :: === === === === === :: === === === === === ::>\n"));
  	Serial.print(F("<:: === === === === === :: === === === === === :: === === PRO === === :: === === === === === :: === === === === === ::>\n"));
  	Serial.print(F("<:: === === === === === :: === === === === === :: "));
  	Serial.print("\t");
  	Serial.print(p.flag);
  	Serial.print("\t");
  	Serial.print(p.temperature); 
  	Serial.print("\t");
  	Serial.print(p.humidity); 
  	Serial.print("\t");
  	Serial.print(p.lightDuration); 
  	Serial.print("\t");
  	Serial.print(p.timestamp); 
  	Serial.print("\t"); 
  	Serial.print(F(" :: === === === === === :: === === === === === ::>\n"));
  	Serial.print(F("<:: === === === === === :: === === === === === :: === === === === === :: === === === === === :: === === === === === ::>\n"));
  }
	// Promotion and Demotion ( maybe put this in a function)
#if ARDUINO == 0
	if(m.flag == 0) {
		heartbeatCounter++;
	} else {
		heartbeatCounter = 0;
		if(isMaster) {
			Serial.print(F("[OK] :: now in Slave mode \n"));
		}
		isMaster = 0;
	}

	if(heartbeatCounter > 60) {
		if(heartbeatCounter == 61) {
			Serial.print(F("[WAR] :: now in Master mode \n"));
		}
		isMaster = 1;
	}
#endif

 	raspiFlag = manageReceive(&p, &toDo); 

	// Set timestamp for this loop
	timestamp = now();

 	if(toDo.initMemory == 1) {
 		initMemory();
 		toDo.initMemory = 0;
 	}

#if ARDUINO==1
	recvNappelSensors( &nappelSensorData, &m);
#else
	if(!isMaster) {
		recvCommands( &actorsCommands , &m);
	}
#endif
	
	if(readSensors(&rawSensorData, &statusOfSensors, ARDUINO)) {
		Serial.print(F("[ERR] :: readSensors failed\n")); 
	}

	if((raspiFlag & 0x8) == 8){
		if (manageProfile(&currentProfile, raspiFlag)) {
			Serial.print(F("[ERR] :: profile invalid\n"));
		}
	}

 
  // reads profile which is in EEPROM because that profile is by design right
  readProfileFromMemory(&currentProfile);
	/* - - - 
	CALCULATION 
	- - - */
#if ARDUINO == 1
	calculateRealCondition(&rawSensorData, &nappelSensorData, &realCondition, &statusOfSensors, &statusOfSensorsNappel);
#endif

#if ARDUINO == 0
	if(isMaster) {
#endif
		if(actorCalculation(&currentProfile, &realCondition, &actorsCommands, timestamp)) {
			Serial.print(F("[ERR] :: actorCalculation failed\n"));
		}
#if ARDUINO == 0
	}
#endif

	/* - - -
	OUTPUT 
	- - - */
 
	if(sendCounter > 13) {
#if ARDUINO==1	
		prepareCommandsToSend(&actorsCommands, &m);
		send_measurements('S', &m, ARDUINO);
#else
		if(!isMaster) {		
			sendNappelSensors(&rawSensorData, &m, &statusOfSensors, timestamp);
			send_measurements('M', &m, ARDUINO);
		}
#endif
		sendCounter = 0;
		Serial.print(F("[OK] :: sending Data to Arduino"));
		Serial.print("\t\t");
		Serial.print(m.flag);
		Serial.print("\t");
		Serial.print(m.temperature); 
		Serial.print("\t");
		Serial.print(m.humidity); 
		Serial.print("\t");
		Serial.print(m.brightness); 
		Serial.print("\t");
		Serial.print(m.moisture);
		Serial.print("\t"); 
		Serial.print(m.timestamp);	
		Serial.print("\n");
	} else {
		sendCounter++;
	}


	actorsControl(&actorsCommands);

	if(timestamp-lastTime2 < RATE_OF_SAVES){
		lastTime2 = timestamp;

		if(saveData(rawSensorData, actorsCommands, timestamp, ARDUINO)) {
			Serial.print(F("[ERR] :: unable to save Data\n"));
		}
	}
	// if(timestamp-lastTime < RATE_OF_SENDS){
	if(sendCounterP > 17) {
		prepareData(&rawSensorData, &actorsCommands, &statusOfSensors, ARDUINO, timestamp, &m);
		// sends data to Raspi
		Serial.print(F("[OK] :: sending Data to Raspberry Pi"));
		Serial.print("\t\t");
		Serial.print(m.flag);
		Serial.print("\t");
		Serial.print(m.temperature); 
		Serial.print("\t");
		Serial.print(m.humidity); 
		Serial.print("\t");
		Serial.print(m.brightness); 
		Serial.print("\t");
		Serial.print(m.moisture);
		Serial.print("\t"); 
		Serial.print(m.timestamp);	
		Serial.print("\n");
		send_measurements('P', &m, ARDUINO);
		// lastTime = timestamp;
		sendCounterP = 0;
	} else {
		sendCounterP++;
	}

 	if(toDo.sendAll == 1) {
 		if(dataSet<MAX_SETS_OF_DATA) {
	 		sendEverything(ARDUINO, dataSet);
	 		dataSet++;
 		} else {
 			toDo.sendAll = 0;
 			dataSet = 0;
 		}
 	}

	wdt_reset();
}



