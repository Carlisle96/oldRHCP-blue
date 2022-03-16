#include <Arduino.h>
#include <SoftwareSerial.h> //Software Serial Port
#include "communication.h"
#include "blauFundamentals.h"
#include "QueueList.h"

#define RxD 2
#define TxD 3
#define BAUD 9600			// baud of bluetooth
#define SERIAL_BAUD 115200  // baud for debugging
#define DEBUG_MODE 1


// N = not connected A=arduino P = pi
char bt_status = 'N';
char bt_master = 'S';
extern char recv_str[100];
int heartbeat = 0;

measurement last_send_measurement;
char last_receiver;
unsigned char last_send_package = 10;

typedef struct sendPackage {
  measurement mea;
  char receiver;
  unsigned char id;
} sendPackage;

QueueList<sendPackage*> sendBuffer = QueueList<sendPackage*>();

int time = 0;
int sended_times = 0;
extern SoftwareSerial btserial; // the software serial port


int send_measurements(char receiver, measurement* m, unsigned char id, int big_boss){

  sendPackage* package = new sendPackage;
  package->mea.flag = m->flag;
  package->mea.temperature = m->temperature;
  package->mea.humidity = m->humidity;
  package->mea.brightness = m->brightness;
  package->mea.moisture = m->moisture;
  package->mea.timestamp = m->timestamp;
  package->receiver = receiver;
  package->id = id;
  sendBuffer.push(package);

  return 0;
}

int initLastMeasurement() {

  if(sendBuffer.isEmpty()) {
    last_send_package = 10;
    return 0;
  }

  sendPackage* temp = sendBuffer.pop();
  last_send_package = temp->id;
  last_receiver = temp->receiver;
  last_send_measurement.flag = temp->mea.flag;
  last_send_measurement.temperature = temp->mea.temperature;
  last_send_measurement.humidity = temp->mea.humidity;
  last_send_measurement.brightness = temp->mea.brightness;
  last_send_measurement.moisture = temp->mea.moisture;
  last_send_measurement.timestamp = temp->mea.timestamp;

  delete temp;
  return 0;

}

int writeBtMeasurement(int big_boss) {
  #if DEBUG_MODE
  Serial.print(F("[WAR] :: send last_sent_measurement:\t"));
  Serial.print("\t");
  Serial.print(last_send_measurement.flag);
  Serial.print("\t");
  Serial.print(last_send_measurement.temperature); 
  Serial.print("\t");
  Serial.print(last_send_measurement.humidity); 
  Serial.print("\t");
  Serial.print(last_send_measurement.brightness); 
  Serial.print("\t");
  Serial.print(last_send_measurement.moisture);
  Serial.print("\t"); 
  Serial.print(last_send_measurement.timestamp);
  Serial.print("\t"); 
  Serial.print(last_send_measurement.outer_temperature);
  Serial.print("\n");
  #endif

  send_header(last_receiver, 'M', last_send_package, big_boss);
  btserial.println(last_send_measurement.flag);
  btserial.println(last_send_measurement.temperature);
  btserial.println(last_send_measurement.humidity);
  btserial.println(last_send_measurement.brightness);
  btserial.println(last_send_measurement.moisture);
  btserial.println(last_send_measurement.timestamp);
  return 0;

}

int send_measurements(char receiver, measurement* m, int big_boss){ return send_measurements(receiver, m, NULL, big_boss);}
//___________________________MAIN METHODS______________________________________


int init_blauzahn(int big_boss){
	#if DEBUG_MODE
		Serial.begin(SERIAL_BAUD); // Serial port for debugging
	#endif
		pinMode(RxD, INPUT);  // UART pin for Bluetooth
		pinMode(TxD, OUTPUT); // UART pin for Bluetooth

		while (setup_blauzahn(big_boss) != 0) {
		  Serial.print(F("[WAR] :: No Connection yet! \n"));
		}
		Serial.print(F("[OK] :: Bluetooth setup was successfull! \n"));
    bt_status = 'N';
	return 0;
}


/*
	wait some seconds
	clear buffer by recv
	check if connected with raspberry
		if not set master/slave connect with ardu (set bt_master)
		(else heartbeat to everyone)
	check for receiving data
*/
int communication_loop(int big_boss, measurement* measure, recv_profileT* prof){
  time++;
	delay(400);
	recv_data(400, big_boss, measure, prof);


	//check if pi is not connected and ardu is master
	if((bt_status == 'N' || (bt_status == 'A' && time%20==0)) && big_boss == 1){
    if(time%20==0){
      disconnect();
      sendATCom("AT+ROLB0");
      sendATCom("AT+RESET");
      bt_status = 'N';
      delay(3100);
      recv_string(400);
      bt_master = 'S';
      
    }else if(time%20==10){
     //not connected
      disconnect();
      if(bt_status != 'P'){
        sendATCom("AT");
        recv_string(400);
        sendATCom("AT+ROLB1");
        sendATCom("AT+RESET");
        delay(3100);
        recv_string(400);
        bt_master = 'M';
      }
    }  
  }else if((time==100 || time==50) && bt_status=='P'){
    if(heartbeat != 0)
      heartbeat = 0;
    else{
      sendATCom("AT");
      bt_status = 'N';
    }
    //i = 0;
  }
    if(time==100)
      time = 0;

  //check if a package is not ack
  if(last_send_package != 10 && (( sended_times != 5 && last_receiver == 'P') || (sended_times != 2 && last_receiver != 'P'))){
    writeBtMeasurement(big_boss);
    sended_times++;
  } else {
    initLastMeasurement();
    sended_times = 0;
  }

	return 0;
}

//___________________________SEND RECV_________________________________________
/* just send some data if it isn't something we can do better by our own methods
      ATTENTION: DATA has to terminate with \0

      Testpackage:
      _SMa(M|P|A)
*/
int send_header(char receiver, char type, char id, int big_boss){
		unsigned char i;
    while(id == NULL || id==10 || id==13)
		  id = random(1,255);

		//get sender
		char sender = big_boss == 1 ? 'M' : 'S';

    btserial.print('_');
		//send sender
		btserial.print(sender);
		//send receiver
		btserial.print(receiver);
		//send id
		btserial.print(id);
		//send type
		btserial.print(type);

    #if DEBUG_MODE
      Serial.print(F("[snd] :: _"));
      Serial.print(sender);
      Serial.print(receiver);
      Serial.print(id);
      Serial.print(type);
      Serial.print('\n');
    #endif

    if(type != 'A'){
      last_send_package = id;
    }

		return 0;
}
int send_header(char receiver, char type, int big_boss){ return send_header(receiver, type, NULL, big_boss);}
