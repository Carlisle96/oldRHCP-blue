#include "datastructures.h"
#include "Arduino.h"
#include "Time.h"
#include "EEPROM.h"
#define CHECKTIME 36

//borders for activating fan or heater
int threshold = 1;
int brightnessBorder = 700;
int rueckgabe = -1;

int actorCalculation(profile *currentProfile, boxCondition *realSensorData, actors *actorsCommands, uint32_t timestamp){
  uint32_t timeOn = timestamp;
    //fan on, heater off
	if (realSensorData->temperature > currentProfile->temperature + threshold){

		actorsCommands->fan = true;
		actorsCommands->heater = false;
    rueckgabe = 0;

	}else if(realSensorData->temperature < currentProfile->temperature - threshold){

  	//fan off, heater on
		actorsCommands->fan = false;
		actorsCommands->heater = true;
    rueckgabe = 0;
    
	}else{

 	// everything off
		actorsCommands->fan = false;
		actorsCommands->heater = false;
    rueckgabe = 0;
	}
  
  if(hour(timestamp) >= 5){;
    if((hour(timestamp)-5) < (currentProfile->lightDuration)){   
        if(realSensorData->brightness < brightnessBorder){
          if(timestamp-timeOn < CHECKTIME){
            if(actorsCommands->light != 2){           
              actorsCommands->light = 2;
              timeOn == timestamp;
            }else{
              actorsCommands->light = 0;
            }
          }else{
            actorsCommands->light = 0;
          }
        }
    }else{
      actorsCommands->light = 0;;
    }
  }
	return rueckgabe;
	}	


// Takes all available data out of all sources and decides the "real condition" inside the Box which all decicions will be based upon
// Expects 3 conditions : Bigboss, Nappel, Real
int calculateRealCondition(boxCondition *rawSensorData, boxCondition *nappelSensorData, boxCondition *realCondition,sensorStatus* statusOfSensors, sensorStatus* statusOfSensorsNappel) {
  // check if all senseful boundaries are met CASE TEMPERATURE must be in (0,40)
  if((rawSensorData->temperature>0 && rawSensorData->temperature <40)&&(nappelSensorData->temperature>0 && nappelSensorData->temperature <40)){
    realCondition->temperature= (rawSensorData->temperature + nappelSensorData->temperature)/2;
  }else if((rawSensorData->temperature>0 && rawSensorData->temperature <40)){
    realCondition->temperature=rawSensorData->temperature;
    statusOfSensorsNappel->tempHum=0;
  }else if ((nappelSensorData->temperature>0 && nappelSensorData->temperature <40)){
    realCondition->temperature=nappelSensorData->temperature;
    statusOfSensors->tempHum=0;
  }

  // check if all senseful boundaries are met CASE HUMIDITY must be in (0,100)
  if((rawSensorData->humidity>0 && rawSensorData->humidity <100)&&(nappelSensorData->humidity>0 && nappelSensorData->humidity<100)){
    realCondition->humidity= (rawSensorData->humidity + nappelSensorData->humidity)/2;
  }else if((rawSensorData->humidity>0 && rawSensorData->humidity <100)){
    realCondition->humidity=rawSensorData->humidity;
    statusOfSensorsNappel->tempHum=0;
  }else if ((nappelSensorData->humidity>0 && nappelSensorData->humidity <100)){
    realCondition->humidity=nappelSensorData->humidity;
    statusOfSensors->tempHum=0;
  }
  
  // check if all senseful boundaries are met CASE BRIGHTNESS must be in (0,2^11-1)
  if(rawSensorData->brightness>0 && rawSensorData->brightness<((1<<11)-1)){
    realCondition->brightness=rawSensorData->brightness;

  }else{// we can anyway not check if nappel sensor brightness is senseful so we just put it in, if Master Sensor is defect
    realCondition->brightness=nappelSensorData->brightness;
    statusOfSensors->brightness=0;
  }

  // Check if Moisture from Nappel is senseful must be in (0,2^10-1)
  if(nappelSensorData->moisture>0&& nappelSensorData->moisture<((1<<10)-1)){
    realCondition->moisture=nappelSensorData->moisture;
  }else{
    statusOfSensorsNappel->moisture=0;
  }
  // Check outer Temperature is in bounds -> decided not to do here and its raspy so we cant set it here anyway
  


return 0;






}









