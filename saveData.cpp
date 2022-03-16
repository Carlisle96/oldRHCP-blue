// library for the static memory
#include <stdint.h>
#include "EEPROM.h"
#include "datastructures.h"
#include "communication.h"
#include "functions.h"
#include "stdio.h"
#include "Arduino.h"

#define BEGIN_OF_DATA 9
#define RATE_OF_SAVE 15
#define MAX_SETS_OF_DATA 203

// Clear the whole EEPROM memory it especially sets the dataWritePoint to 0 aswell - also sets the defaultProfile into EEPROM
// 0 - dataWritePoint
// 1 - 3 profile
// 4 - x timestamp
int initMemory() {

	int i;
	for(i=0; i < 1024; i++) {
		EEPROM.write(i, 0);
	}

	profile defaultProfile;
	defaultProfile.lightDuration = 13;
	defaultProfile.temperature = 26;
	defaultProfile.humidity	= 50;
	saveProfile(defaultProfile);

	return 0;
}

int saveProfile(profile profile) {

	int i;

	// check for boundaries
	if(profile.lightDuration < 0 || profile.lightDuration > 24 || profile.temperature < 0 || 
		profile.temperature > 51.2 || profile.humidity > 100 || profile.humidity < 0 ) {
		return -1;
	}
	
	uint8_t profileBuffer[3];

	unsigned int lightDuration = (unsigned int)(profile.lightDuration*10);
	unsigned int temperature = (unsigned int)(profile.temperature*10);
	unsigned int humidity = (unsigned int)(profile.humidity);

	profileBuffer[0] = (uint8_t)lightDuration;
	profileBuffer[1] = (uint8_t)(temperature>>1);
	profileBuffer[2] = (uint8_t)(temperature<<8) | ((uint8_t)profile.humidity & 0x7F);

	for(i=0; i < 3; i++) {
		EEPROM.write(1 + i, profileBuffer[i]);
	}

	return 0;

}

int readProfileFromMemory(profile* profile) {

	int i;

	uint8_t profileBuffer[3];

	for(i=0; i < 3; i++) {
		profileBuffer[i] = EEPROM.read(1 + i);
	}

	profile->lightDuration = (float)profileBuffer[0]/10;
	profile->temperature = (float)((unsigned int)profileBuffer[1]<<1 | profileBuffer[2]>>8)/10;
	profile->humidity = (float)(profileBuffer[2] & 0x7F);

	return 0;
}

/*
the values are being converted into integer format and then merged together into a one big chunk
before being stored in the EEPROM 
- - -
5 bytes (40 bits) per datapoint (you can only adress the EEPROM by whole bytes)
		temperature	humidity	actors	brightness	moisture	light	total
bigbos	11			10		 	2		14			0			2		39 / 40
nappel	11			10		 	2		1			10			2		36 / 40
- - -
We can save 203 datapoints by this method starting at byte 9. The first 9 bytes are used for overhead such as storing
the current tail of the data and the timestamp and the storage of the profile. 
*/
int saveData(boxCondition sensorData, actors actorData, uint32_t timestamp, int arduino) {

	// check for boundaries
	if(sensorData.temperature < -100 || sensorData.temperature > 104.7 || sensorData.humidity < 0 || 
		sensorData.humidity > 100 || sensorData.moisture > 1024 || actorData.light > 2 ) {
		return -1;
	}

	int i;

	// The # of the dataset which is next to be overwritten ( it is always stored in EEPROM memory in location 0 )
	// To get the address you should calculate the address = 4 + 5*dataWritePoint
	uint8_t dataWritePoint = EEPROM.read(0);

	// time handleing
	uint8_t numberOfCalls = EEPROM.read(8);

	// update numberofcalls
	if(numberOfCalls <= MAX_SETS_OF_DATA) {
		numberOfCalls++;
    EEPROM.write(8,numberOfCalls);
	}

	if(numberOfCalls == 1) {
		for(i=0; i<4; i++) {
			EEPROM.write(4+i, (uint8_t)(timestamp>>(24-8*i)));
		}
	} else if(numberOfCalls > MAX_SETS_OF_DATA) {
		uint32_t timestamp = 0;
		for(i=0; i<4; i++) {
			timestamp |= (uint32_t)EEPROM.read(4+i)<<(24-8*i);
		}
		timestamp += 60*RATE_OF_SAVE;
		for(i=0; i<4; i++) {
			EEPROM.write(4+i, (uint8_t)(timestamp>>(24-8*i)));
		}		
	}
	// getting the relevant digits - for the temperature add an offset 100 to only hold positive numbers
	unsigned int temperature = (unsigned int)(sensorData.temperature*10 + 1000);
	unsigned int humidity = (unsigned int)(sensorData.humidity*10);

	// Stuffing everything into the 5 bytes we want to store
	uint8_t dataBuffer[5];

	if(arduino) {

		dataBuffer[0] = (uint8_t)(temperature >> 3);
		dataBuffer[1] = (uint8_t)temperature << 5 | (uint8_t)(humidity >> 5);
		dataBuffer[2] = (uint8_t)humidity << 3 | actorData.fan << 1 | actorData.heater;
		dataBuffer[3] = (uint8_t)(sensorData.brightness >> 6);
		dataBuffer[4] = (uint8_t)sensorData.brightness << 2 | actorData.light;

	} else {

		// this is a temporary fix => the analog brightness sensors worked differently than we expected when save was implemented 
		// the way data is stored could be reworked to take the new format into account
		int brightness;
		if(sensorData.brightness >= 512) {
			brightness = 1;
		} else {
			brightness = 0;
		}

	    dataBuffer[0] = (uint8_t)(temperature >> 3);
	    dataBuffer[1] = (uint8_t)temperature << 5 | (uint8_t)(humidity >> 5);
	    dataBuffer[2] = (uint8_t)humidity << 3 | actorData.fan << 1 | actorData.heater;
	    dataBuffer[3] = (uint8_t)(sensorData.moisture>>3)&0x7F | brightness<<7 & 0x80;
	    dataBuffer[4] = (uint8_t)sensorData.moisture << 5 | actorData.light;

	}

	// Write the buffer to EEPROM
	for(i=0; i < 5; i++) {
		EEPROM.write(BEGIN_OF_DATA + 5*dataWritePoint + i, dataBuffer[i]);
	}

	// update head of EEPROM
	EEPROM.write(0,(dataWritePoint+1)%(MAX_SETS_OF_DATA));

	return 0;

}

uint32_t readDataFromMemory(boxCondition* sensorData, actors* actorData, uint8_t dataReadPoint, int arduino) {

	/*	
	if(dataReadPoint > 202) {

	}
	*/

	int i;
	uint8_t dataBuffer[5];

	for(i=0; i < 5; i++) {
		dataBuffer[i] = EEPROM.read(BEGIN_OF_DATA + 5*dataReadPoint + i);
	}

	if(arduino) {
		
		sensorData->temperature = (float)((unsigned int)dataBuffer[0] << 3 | (unsigned int)dataBuffer[1] >> 5)/10-100;
		sensorData->humidity = (float)((unsigned int)(dataBuffer[1] & 0x1F) << 5 | (unsigned int)dataBuffer[2] >> 3)/10;
		sensorData->brightness = (unsigned int)dataBuffer[3] << 6 | (unsigned int)dataBuffer[4] >> 2;
		actorData->light = dataBuffer[4] & 0x3;
		actorData->fan = dataBuffer[2] >> 1 & 0x1;
		actorData->heater = dataBuffer[2] & 0x1;

	} else {

		sensorData->temperature = (float)((unsigned int)dataBuffer[0] << 3 | (unsigned int)dataBuffer[1] >> 5)/10-100;
		sensorData->humidity = (float)((unsigned int)(dataBuffer[1] & 0x1F) << 5 | (unsigned int)dataBuffer[2] >> 3)/10;
		sensorData->brightness = (unsigned int)(dataBuffer[3] >> 7);
		sensorData->moisture = (unsigned int)(dataBuffer[3]&0x7F) << 3 | dataBuffer[4] >> 5;
		actorData->light = dataBuffer[4] & 0x3;
		actorData->fan = dataBuffer[2] >> 1 & 0x1;
		actorData->heater = dataBuffer[2] & 0x1;

	}

	uint32_t timestamp = 0;
	for(i=0; i<4; i++) {
		timestamp |= (uint32_t)EEPROM.read(4+i)<<(24-8*i);
	}

	uint8_t dataWritePoint = EEPROM.read(0);

	if(EEPROM.read(8) <= MAX_SETS_OF_DATA) {
		timestamp += 60*RATE_OF_SAVE*dataReadPoint;
	}
	else {
		timestamp += ((dataReadPoint-dataWritePoint+(MAX_SETS_OF_DATA))%(MAX_SETS_OF_DATA))*60*RATE_OF_SAVE;
	}
	
	return timestamp;
}


uint8_t sendEverything(int arduino, int i) {
	
	boxCondition sensorBuffer;
	actors actorBuffer;
	sensorStatus statusBuffer;
  	statusBuffer.tempHum = 1;
  	statusBuffer.moisture = 1;
  	statusBuffer.brightness = 1;

  	measurement measurements;
	uint32_t timestamp = readDataFromMemory(&sensorBuffer, &actorBuffer, i, arduino); 
	prepareData(&sensorBuffer, &actorBuffer, &statusBuffer, arduino, timestamp, &measurements);
	send_measurements('P', &measurements, arduino);
	return 0;
}

// stackoverflow copycat
void getBin(int num, char *str) {
	*(str+8) = '\0';
	int mask = 0x80 << 1;
	while(mask >>= 1) {
		*str++ = !!(mask & num) + '0';
	}
}

void printMemory(int maxMemory) {

	int i;
	char printBuffer[9] = "00000000";

	Serial.print("\n\t\t\t\t\t\t ::: Memory :::\n");
	Serial.print("dwp\t");
	for(i=0; i<maxMemory; i++) {
		if( i == 0 ) {
			Serial.print(EEPROM.read(i));
		} else {
			getBin(EEPROM.read(i), printBuffer);
			Serial.print(printBuffer);
		}
		Serial.print("\t");
		if( i == 0 || i == 3 || i == 8 || (i-3)%5 == 0 ) {
			Serial.print("\n");
			if(i == 0) {
				Serial.print("prf");
			}
			if(i == 3) {
				Serial.print("tms");
			}
			if(i >= 8 && i < maxMemory-1) {
				Serial.print((i-8)/5);				
			}
			Serial.print("\t");
		}
	}
	Serial.print("\t\t\t\t\t ::: - -- - :::\n");

}
