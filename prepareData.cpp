#include "datastructures.h"
#include "Arduino.h"
#include "functions.h"
#include "communication.h"


int prepareData(boxCondition* sensorData, actors* actorData, sensorStatus* sensorStatus, int arduino, uint32_t timestamp, measurement* measurements){

  //sets flagInt
  unsigned int flag = 0;
  flag += arduino;
  flag += (actorData->light << 1);
  flag += (actorData->fan << 3);
  flag += (actorData->heater << 4);
  flag += (sensorStatus->moisture << 5);
  flag += (sensorStatus->tempHum << 6);
  flag += (sensorStatus->brightness << 7);
  if(arduino == 0){
    flag += 256;
  }
  if(!(sensorStatus->moisture)){
    flag += 512;
  }
  if(!(sensorStatus->tempHum)){
    flag += 1024;
  }
  if(!(sensorStatus->brightness)){
    flag += 0; //brightness to be done
  }
  
  /*Serial.print(flag);
  Serial.print("\n"); 
  Serial.print(actorData->heater);
  Serial.print("\n"); */

  // prepares the data for sending
  measurements->flag = flag;
  measurements->temperature = sensorData->temperature; 
  measurements->humidity = sensorData->humidity;
  measurements->brightness = sensorData->brightness;
  measurements->moisture = sensorData->moisture;
  measurements->timestamp = timestamp;

  return 1;
}

