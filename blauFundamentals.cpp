#include <Arduino.h>
#include <SoftwareSerial.h> //Software Serial Port
#include "blauFundamentals.h"

#include "communication.h"
#define RxD 2
#define TxD 3

#define MAC1 "000E0B0A4AD5" // MAC of BLE 1 (00:0E:0B:0A:4A:D5)
#define MAC2 "000E0B0B60E8" // MAC of BLE 2 (00:0E:0B:0B:60:E8)
#define MAC_PI "B827EB5782D6" //MAC of Raspberry
#define BAUD 9600			// baud of bluetooth
#define SERIAL_BAUD 115200  // baud for debugging
#define DEBUG_MODE 1

SoftwareSerial btserial(RxD, TxD); // the software serial port
/*
	0 = MAC1
	1 = MAC2
	-1 = no modul
*/
char recv_str[100]; //default buffer of recv_string
int i_profile = 0;
int i_measurement = 0;
char sender;
char receiver;
char type;
char packageNo;
extern unsigned char last_send_package; //id of last package

extern char bt_master; //state of bluetoothmodul
extern char bt_status; //state of bluetooth ('N' not connected 'P' connected with pi 'A' connected with ardu)
extern int heartbeat; //heartbeat
//------------------------------------------------------------------------------

// functions to get global variables
int getData(measurement* me, recv_profileT* pr) {
  
  me->flag = m.flag;
  me->temperature = m.temperature;
  me->humidity = m.humidity;
  me->brightness = m.brightness;
  me->moisture = m.moisture;
  me->timestamp = m.timestamp;
  me->outer_temperature = m.outer_temperature;

  pr->flag = p.flag;
  pr->temperature = p.temperature;
  pr->humidity = p.humidity;
  pr->lightDuration = p.lightDuration;
  pr->timestamp = p.timestamp;

}

/*
	Setup valid bluetooth connection with given constants
*/
int setup_blauzahn(int big_boss){
	delay(3500); //wait for module restart

	//check on baudrates baud and serial_baud (these are standard on grove-ble)
	while(1){
		if(sendAT(BAUD)==0) break;

		if(sendAT(SERIAL_BAUD)==0) break;
	}

//we have to set the baud rate to 9600, since the soft serial is not stable at 115200
  sendATCom("AT");
  sendATCom("AT+RENEW");//restore factory configurations
  sendATCom("AT");
  sendATCom("AT+BAUD2");//reset the module's baud rate
  sendATCom("AT");
  sendATCom("AT+AUTH1");//enable authentication
  sendATCom("AT");
  sendATCom("AT+RESET");//restart module to take effect
  btserial.begin(9600);//reset the Arduino's baud rate
  delay(3500);//wait for module restart
//set everything
  if(big_boss==1){
    sendATCom("AT");
    sendATCom("AT+NAMBRED-HOT-BLE1");//set BLE name
  }else{
    sendATCom("AT");
    sendATCom("AT+NAMBRED-HOT-BLE0");
  }
  sendATCom("AT");
  sendATCom("AT+PINE9526");//set EDR password
  sendATCom("AT");
  sendATCom("AT+PINB9526");//set BLE password
  sendATCom("AT");
  sendATCom("AT+SCAN0");//set visible
  sendATCom("AT");
  sendATCom("AT+NOTI1");//enable Connection Notification
  sendATCom("AT");
  sendATCom("AT+NOTP1");//enable address Notification
  sendATCom("AT");
  sendATCom("AT+DUAL0");//spp or ble connections
  sendATCom("AT");
  sendATCom("AT+MODE1");//transfer data and response at commands without connection
  sendATCom("AT");
  sendATCom("AT+ROLB0");//set to slave mode
  sendATCom("AT");
  sendATCom("AT+RESET");//restart module to take effect
	delay(3500);//wait for module restart

	if(sendAT(BAUD) == 0){  //check if device is online
		if(recv_str[0]=='O' && recv_str[1]=='K'){
			#if DEBUG_MODE
				Serial.print("[OK]");
			#endif
			return 0;
		}else{
			#if DEBUG_MODE
				Serial.print("[ERR] :: last reset wasn't working");
			#endif
		}
	}

	return -1;
}

/* restart modul with all wanted settings
      0 everything is fine
      -1 someting went wrong
*/
int disconnect(){
	return sendAT(BAUD);
}

//----------------------------------------------------------------------------

int recv_profile(measurement* measure, recv_profileT* prof){
	#if DEBUG_MODE
  	Serial.println(F("[OK] :: Profile received"));
	#endif
	String inString = "";

  int recv = 0;
  int i = i_profile;
  int time = 0;
	//check for some time for all data to receive
  while (time < 100 && i<6){
    delay(50);

    while (btserial.available() > 0 && i < 6) {
      int inChar = btserial.read();

      if (inChar != '\n') {

        // As long as the incoming byte
        // is not a newline,
        // convert the incoming byte to a char
        // and add it to the string
        inString += (char)inChar;
      }
      // if you get a newline, print the string,
      // then the string's value as a float:
      else {
        switch (i) {
          case 0:
          prof->flag = inString.toInt();
          break;
          case 1:
          prof->temperature = inString.toFloat();
          break;
          case 2:
          prof->humidity = inString.toFloat();
          break;
          case 3:
          prof->lightDuration = inString.toInt();
          break;
	        case 4:
	        prof->timestamp = (unsigned long) atol(inString.c_str());
	        break;
          case 5:
          measure->outer_temperature = inString.toFloat();
          break;
        }
        i++;
        inString = "";
      }
    }
    time++;
  }
	
	if(i == 6){
		i_profile = 0;
    recv = 1;
	}else
		i_profile = i;

	#if DEBUG_MODE
	  Serial.println(F("------------------"));
	  Serial.println(prof->flag);
	  Serial.println(prof->temperature);
	  Serial.println(prof->humidity);
	  Serial.println(prof->lightDuration);
	  Serial.println(measure->outer_temperature);
	#endif
  return recv;
}

int recv_measurements(measurement* measure, recv_profileT* prof){
	#if DEBUG_MODE
  	Serial.println(F("[OK] :: Measurement received"));
	#endif
  String inString = "";

  int recv = 0;
  int time = 0;
  while (time < 400/50 && i_measurement<6){
    delay(50);

    while (btserial.available() > 0 && i_measurement < 6) {
      int inChar = btserial.read();

      if (inChar != '\n') {

        // As long as the incoming byte
        // is not a newline,
        // convert the incoming byte to a char
        // and add it to the string
        inString += (char)inChar;
      }
      // if you get a newline, print the string,
      // then the string's value as a float:
      else {
        switch (i_measurement) {
          case 0:
          measure->flag = inString.toInt();
          break;
          case 1:
          measure->temperature = inString.toFloat();
          break;
          case 2:
          measure->humidity = inString.toFloat();
          break;
          case 3:
          measure->brightness = inString.toInt();
          break;
          case 4:
          measure->moisture = inString.toInt();
          break;
          case 5:
          measure->timestamp = (unsigned long) atol(inString.c_str());
          break;
        }
        i_measurement++;
        inString = "";
      }
    }
    time++;
  }

	if(i_measurement == 6){
		i_measurement = 0;
    recv = 1;
	}

	#if DEBUG_MODE
	  Serial.println(F("------------------"));
	  Serial.println(measure->flag);
	  Serial.println(measure->temperature);
	  Serial.println(measure->humidity);
	  Serial.println(measure->brightness);
	  Serial.println(measure->moisture);
	  Serial.println(measure->timestamp);
	#endif

  return recv;
}

/*recv any data*/
int recv_data(unsigned int timeout, int big_boss, measurement* measure, recv_profileT* prof){
	unsigned int time = 0;
	unsigned char i;
	unsigned char buf[100];
	int available = 0;

	//wait some time for the first character
	i = 0;
	while(1){
		delay(50);

		//if there is a a charakter readable without timeout keep on
    //Serial.println(blueToothSerial.available());
		if (btserial.available() != 0) {
			available = btserial.available();
		  break;
		}
		time++;

		//check if timeout took place
		if (time > (timeout / 50)) {
		  return -1;
		}
	}

  //recv packages until buffer is empty
  while(available != 0){
    if(btserial.peek() != '_' && i_profile==0 && i_measurement==0)
      return recv_string(400);

    available = btserial.available();
		//recv header
		if(i_profile==0 && i_measurement==0){
		  char unimportant = char(btserial.read());
			sender = char(btserial.read());
			receiver = char(btserial.read());
			packageNo = (unsigned char) btserial.read();
			type = char(btserial.read());

		#if DEBUG_MODE
	    Serial.print(F("[OK] :: sender "));
	    Serial.print(sender);
	    Serial.print(F("  :: receiver "));
	    Serial.println(receiver);
	    Serial.print(F("[OK] :: packageNo "));
	    Serial.print((char)packageNo);
	    Serial.print(F(" :: type "));
	    Serial.print(type);
		Serial.print('\n');
	  #endif

			// check if right receiver if not quit and flush all recv data
			if(big_boss == 1){
				if(receiver != 'M')
					continue;
			}else{
				if(receiver != 'S')
					continue;
			}

      int recved = 0;

			//get type of message -> unpack
			if(type=='A'){
		    recv_ack();
		    continue;
			}else if(type == 'M')
				recved += recv_measurements(measure, prof);
			else if(type == 'P')
				recved += recv_profile(measure, prof);

      if(recved != 0){
	if(bt_status == 'P')
		heartbeat++;
        //send ack with packageno
        String ack = "ACK";
        ack += char(packageNo);
        send_header(sender, 'A', big_boss);
        btserial.println(ack);
        #if DEBUG_MODE
          Serial.println(ack);
        #endif
      }
			


		}else if(i_profile!=0){
		//if profile was not recv totally continue
			recv_profile(measure, prof);

		}else if(i_measurement!=0){
		//if measurement was not recv totally continue
			recv_measurements(measure, prof);
		}

    available = btserial.available();
	}
	return 0;
}

/*
recvStrings
*/
int recv_string(unsigned int timeout) {
      unsigned int time = 0;
      unsigned char num;
      unsigned char i;

      // waiting for the first character with time out
      i = 0;
      while (1) {
        delay(50);

        //if there is a a charakter readable without timeout keep on
        //Serial.println(blueToothSerial.available());
        if (btserial.available() != 0) {
          recv_str[i] = char(btserial.read());
          i++;
          break;
        }
        time++;

        //check if timeout took place
        if (time > (timeout / 50)) {
          return -1;
        }
      }

      // read other characters from uart buffer to string
      while (btserial.available() && (i < 100)) {
        if (btserial.peek() == '_'){
          break;
        }
        recv_str[i] = char(btserial.read());
        i++;
      }
      recv_str[i] = '\0';
			#if DEBUG_MODE
	Serial.print(F("[rvd] :: "));
      	Serial.print(recv_str);
	Serial.print('\n');
			#endif

      change_bt_status();
      return 0;
}

/*
	recv ack
*/
int recv_ack(){
	char id = 10;

	for(int i=0;i<4;i++){
    id = btserial.read();
	}
	if((unsigned char) id == last_send_package)
    last_send_package = 10;
	#if DEBUG_MODE
	  Serial.print(F("[OK] :: Ack recv "));
	  Serial.print(char(id));
	#endif
  return 0;
}

/*
	sendATCommands
*/
int sendATCom(char command[]) {

#if DEBUG_MODE
  // send
  Serial.print(F("[snd] :: "));
  Serial.print(command);
  Serial.print("\n");
#endif
  // send bluetoothcommand
  btserial.print(command);
  delay(200);

//something recieved?
  if (recv_string(400) != 0)
		return -1;

  return 0;
}


/*
	sendAT only send at command and check for right answer
*/
int sendAT(long baud){
	delay(500);

	//set baud
	btserial.begin(baud);
	delay(500);
	#if DEBUG_MODE
		Serial.print(F("[WAR] :: try "));
		Serial.println(baud);
	#endif

	if(sendATCom("AT") == 0){  //check if there is a disconnected device
		//check for right answer (OK)-> disconnected old connection (ER)->Error
		//if right answer return 0
		if(recv_str[0]=='O' && recv_str[1]=='K'){
			#if DEBUG_MODE
				Serial.println(F("[OK] :: AT confirmed"));
			#endif
			return 0;
		}else{
			#if DEBUG_MODE
				Serial.println(F("[ERR] :: AT not confirmed"));
			#endif
		}
	}

	return -1;
}

void change_bt_status(){
  if(strstr(recv_str,"LSTB")!=NULL && strstr(recv_str, "LSTB") > strstr(recv_str, "CONB"))
    bt_status = 'N';
  else if(strstr(recv_str,MAC_PI)!=NULL  || strstr(recv_str,"FCCD7ED37386")!=NULL)
    bt_status = 'P';
  else if(strstr(recv_str,MAC1)!=NULL || strstr(recv_str,MAC2)!=NULL)
    bt_status = 'A';
	#if DEBUG_MODE
	  Serial.print(F("[WAR] :: Connection Status: "));
	  Serial.print(bt_status);
    Serial.print("\n");
	#endif
}



