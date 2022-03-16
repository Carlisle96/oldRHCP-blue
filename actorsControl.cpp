#include "datastructures.h"
#include "Arduino.h"
#include "RGBdriver.h"

int fanPin = 5;
int heaterPin = 6;
int lightPin = 7;
int dio = 8;

void actorsControl(actors *actorsCommands){
 
 //control fan 
    if(actorsCommands->fan == true){
        digitalWrite(fanPin, HIGH);
    }else{
        digitalWrite(fanPin, LOW);
    }

// control heater
    if(actorsCommands->heater == true){
        digitalWrite(heaterPin, HIGH);
    }else{
        digitalWrite(heaterPin, LOW);
    }
// controll light
    RGBdriver Driver(lightPin, dio);
    //Serial.print(F("\t\t\t\t\t\t\t\t\t\t actorsCommands->light::: "));
    //Serial.println(actorsCommands->light);
    if(actorsCommands->light == 2){
      Driver.begin();
      Driver.SetColor(255,255,255);
      Driver.end();
    }else{
      Driver.begin();
      Driver.SetColor(0,0,0);
      Driver.end();
    }
}
