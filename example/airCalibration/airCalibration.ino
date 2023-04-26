/*****************************************************************
File:         air Calibration 
Description:  Disable the automatic output of the TX pin, 
              use the serial monitor to send 1 to restore the factory settings. 
              Send 2 for air calibration and send 3 for module reset.
******************************************************************/ 
#include "BM22S2021-1.h"
uint8_t i; 
uint8_t STATUS=5;
BM22S2021_1 SMOKE(STATUS,6,7);//intPin 5,rxPin 6 , txPin 7, Please comment out the line of code if you don't use software Serial
//BM22S2021_1 SMOKE(STATUS,&Serial1);//Please uncomment out the line of code if you use HW Serial1 on BMduino
//BM22S2021_1 SMOKE(STATUS,&Serial2);//Please uncomment out the line of code if you use HW Serial2 on BMduino
//BM22S2021_1 SMOKE(STATUS,&Serial3);//Please uncomment out the line of code if you use HW Serial3 on BMduino
//BM22S2021_1 SMOKE(STATUS,&Serial4);//Please uncomment out the line of code if you use HW Serial4 on BMduino

void setup() {
  Serial.begin(9600);
  SMOKE.begin(); //Module initialization, baud rate 9600
  i=SMOKE.setAutoTx(0x00); //0x00 No output close automatic output
  if(i==0)//You can use this method to check whether other configurations are successful
  {
    Serial.println("Module connected"); 
  } 
}

void loop() {
    if(Serial.available() > 0) 
    {
    i=Serial.read();
    Serial.println(i,HEX);
    }   
    if(i==0x32)//Send 2 to the serial port monitor for air calibration
      {
        Serial.println("Air calibration begin");
        i=SMOKE.calibrateModule();//Wait for 8s
        if(i==0)
        {
        Serial.println("Air calibration successful");
        }
        else
        {
          Serial.println("Air calibration failed");
        }        
      }
      i=0;
}
