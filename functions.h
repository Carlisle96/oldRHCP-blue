#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

int readSensors(boxCondition *rawSensorData, sensorStatus* sensorStatus, int arduino);

int actorCalculation(profile *currentProfile, boxCondition *realSensorData, actors *actorsCommands, uint32_t timestamp);

void actorsControl(actors *actorsCommands);

int testOperations(boxCondition* sensorData, actors* actorData);

int manageProfile(profile *currentProfile, int flag);

int prepareData(boxCondition* rawSensorData, actors* actorsCommands, sensorStatus* sensorStatus, int arduino, uint32_t timestamp, measurement* measurements);

int calculateRealCondition(boxCondition *rawSensorData, boxCondition *nappelSensorData, boxCondition *realCondition,sensorStatus* statusOfSensors, sensorStatus* statusOfSensorsNappel);

// Memory functions
int initMemory();
int saveData(boxCondition sensorData, actors actorData, uint32_t timestamp, int arduino);
uint32_t readDataFromMemory(boxCondition* sensorData, actors* actorData, uint8_t dataReadPoint, int arduino);
int saveProfile(profile profile);
int readProfileFromMemory(profile* profile);
uint8_t sendEverything(int arduino, int i);
void getBin(int num, char *str);
void printMemory(int maxMemory);

// communication Arduinos

int manageReceive(recv_profileT* profile, controller* toDo);
int recvNappelSensors(boxCondition* rawSensorData, measurement* measurements);
int sendNappelSensors(boxCondition* rawSensorData, measurement* measurements, sensorStatus * statusOfSensors, uint32_t timestamp);
int recvCommands(actors* actorCommands , measurement* measurements);
int prepareCommandsToSend(actors *actorCommands, measurement *measurements);


#endif



