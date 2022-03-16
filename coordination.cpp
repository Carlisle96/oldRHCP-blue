// this file contains the functions concerning communication between the arduinos
#include "datastructures.h"
#include "Arduino.h"
#include "Time.h"

// manages toDo ( which is received from Raspi)  and sets the new timestamp if available
int manageReceive(recv_profileT* p, controller* toDo) {
	
	if(p->timestamp != 0) {
		setTime(p->timestamp);
		p->timestamp = 0;
	}
 	
 	if(p->flag & 0x1) {
 		toDo->sendAll = 1;
 		p->flag &= 0xE;
 	}
 	if( (p->flag & 0x2) == 2) {
 		toDo->initMemory = 1;
 		p->flag &= 0xD;
 	}

 	return p->flag;
}


int prepareCommandsToSend(actors *actorCommands, measurement *measurements) {
	measurements->flag = 16 | actorCommands->light<<2 | (uint8_t)actorCommands->fan <<1 | (uint8_t)actorCommands->heater;
	return 0;
}

int recvCommands(actors* actorCommands , measurement* measurements){
	// commands are in flag

	if( measurements->flag>>4 == 1){
		actorCommands->light= (uint8_t)(measurements->flag & 12)>>2;
		actorCommands->fan= (bool)((measurements->flag &2)>>1);
		actorCommands->heater= (bool)(measurements->flag&1);
	}
	measurements->flag=0;
	return 0;
}

int sendNappelSensors(boxCondition* rawSensorData, measurement* measurements, sensorStatus * statusOfSensors, uint32_t timestamp){
	measurements->flag = 8;
	
	if(!statusOfSensors->tempHum){
		measurements->flag |= 4;
	}
	if(! statusOfSensors->brightness){
		measurements->flag |= 2;
	}
	if(! statusOfSensors->moisture){
		measurements->flag |= 1;
	}
	measurements->moisture= rawSensorData->moisture;
	measurements->brightness= rawSensorData->brightness;
	measurements->humidity= rawSensorData->humidity;
	measurements->temperature= rawSensorData->temperature;

	return 0;

}

int recvNappelSensors(boxCondition* rawSensorData, measurement* measurements){
	if(measurements->flag& 8 ==8){
		if(measurements->flag & 4 ==4){
			rawSensorData->humidity= measurements->humidity;
			rawSensorData->temperature= measurements->temperature;
		}
		if(measurements->flag &2 ==2){
			rawSensorData->brightness= measurements->brightness;
		}
		if(measurements->flag &1 ==1){
			rawSensorData->moisture= measurements->moisture;
		}

	}
	measurements->flag=0;
	return 0;
}
