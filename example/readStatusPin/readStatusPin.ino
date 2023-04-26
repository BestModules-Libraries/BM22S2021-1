/*****************************************************************
File:         readStatusPin     
              Description:  Set the module parameters, and read a piece of data automatically output by the module every 8 seconds. 
              The LED will have different actions for different states of the module. 
              a. Module normal: the flag is 0. There is no smoke alarm. Led13 flashes slowly.
              b. Module alarm: flag 1: it is detected that the current smoke concentration in the air exceeds the threshold, and led13 remains on.
******************************************************************/ 
#include "BM22S2021-1.h" 
uint8_t STATUS=5;
uint8_t flag=3; 
BM22S2021_1 SMOKE(STATUS,6,7);//intPin 5,rxPin 6 , txPin 7, Please comment out the line of code if you don't use software Serial
//BM22S2021_1 SMOKE(STATUS,&Serial1);//Please uncomment out the line of code if you use HW Serial1 on BMduino
//BM22S2021_1 SMOKE(STATUS,&Serial2);//Please uncomment out the line of code if you use HW Serial2 on BMduino
//BM22S2021_1 SMOKE(STATUS,&Serial3);//Please uncomment out the line of code if you use HW Serial3 on BMduino
//BM22S2021_1 SMOKE(STATUS,&Serial4);//Please uncomment out the line of code if you use HW Serial4 on BMduino
void setup() {
  Serial.begin(9600);
  SMOKE.begin(); //Module initialization, baud rate 9600
  pinMode(STATUS,INPUT);
  pinMode(13,OUTPUT);
  delay(8000);
}

void loop() {
  if(flag!=0&&SMOKE.getSTATUS()==LOW)
  {
    flag=0;
    Serial.println("Module normal;No alarm"); 
  }
  if(SMOKE.getSTATUS()==HIGH&&flag!=1)
  { 
    Serial.println("Alarm! Smoke concentration too high"); 
    flag=1;
  }
  switch(flag)
  {
    case 0:      
    digitalWrite(13,1);
    delay(100);
    digitalWrite(13,LOW);
    delay(7900);
    break;
    case 1:      
    digitalWrite(13,1);
    delay(8000);
    break;
  }
}
