/* version 1.1*/
#ifndef BLAUFUNDAMENTALS
#define BLAUFUNDAMENTALS

#include "datastructures.h"

/* connect to right mac and do all setup configurations
    big_boss  1 if arduino is big boss
*/
int setup_blauzahn(int big_boss);

/* restart modul with all wanted settings
      0 everything is fine
      -1 someting went wrong
*/
int disconnect();

//----------------------------------------------------

int recv_profile(measurement* measure, recv_profileT* prof);

int recv_measurements(measurement* measure, recv_profileT* prof);

/* recv anything
    -1 timeout
*/
int recv_data(unsigned int timeout, int big_boss, measurement* measure, recv_profileT* prof);

int recv_string(unsigned int timeout);

/* recv ack*/
int recv_ack();

//---------------------------------------------------
/* send an at command*/
int sendATCom(char command[]);

/* send at command "AT" for checking if device available or has error*/
int sendAT(long baud);

void change_bt_status();
#endif
