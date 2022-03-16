#include "datastructures.h"
#include "Arduino.h"
#include "functions.h"
#include "communication.h"


int manageProfile(profile *currentProfile, int flag){
  //flag = recv(currentProfile);
  int mask = 8; 
  if ((flag & mask) == 8){
    if (currentProfile->temperature < 0 || currentProfile->temperature > 32 || currentProfile->lightDuration < 8 || currentProfile->lightDuration > 18 || currentProfile->humidity < 20 || currentProfile->humidity > 60){
      return -1;
    }else{
      saveProfile(*currentProfile);
    }
  }
  readProfileFromMemory(currentProfile);
  flag == flag && 0x7;
  /*
  Serial.print(currentProfile->temperature);
  Serial.print("\n");
  */
  return 0;
}

