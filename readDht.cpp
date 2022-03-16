#include "DHT.h"
#include "datastructures.h"
#include <Wire.h>
#include <Digital_Light_TSL2561.h>
#define DHTPIN A0 
#define MOISTUREPIN A1
//changed to A3 to be able to put it in
// Light sensor bigbos D3
#define LIGHT_SENSOR_NAPPEL A3

/* 
readSensors gets the struct for the actual Sensordata and writes the values into the struct it was given
Expects: a pointer to the rawSensorData struct and an int indicating the arduino
Returns: 0 if succesful; -1 if sensors are broken
*/
int readSensors(boxCondition *rawSensorData, sensorStatus* sensorStatus, int arduino) {

    uint8_t dhtType;
    if(arduino) {
        dhtType = 22;
    } else {
        dhtType = 11;
    }
    DHT dht(DHTPIN, dhtType);
    dht.begin();

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int b;
    int m;

    if(t < 0 || t > 60 || h<0 || h>100) {
        sensorStatus->tempHum = 1;
        return -1;
    }

    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(t) || isnan(h)) {
        return -1;
    } else {
        rawSensorData->temperature = t;
        rawSensorData->humidity = h;
    }

    //brightness sensor
    if(arduino){
        b = TSL2561.readVisibleLux();
    }else{
      b = analogRead(LIGHT_SENSOR_NAPPEL);
    }

    //check only for digital, because don't know shit about analog yet.
    if(b < 0 || b > 50000){
      sensorStatus->brightness = 1;
      return -1;
    }

     if (isnan(b)) {
        return -1;
    } else {
        rawSensorData->brightness = b;
    }
    
    //moi moi sensor!
    if(!(arduino)){
      m = analogRead(MOISTUREPIN);
      if(isnan(m)){
        return -1;
      }else{
        if(m < 0 || m > 940){
          return -1;
        }else{
          rawSensorData->moisture = m;
        }
      }
    }
      
    
    return 0;
}


