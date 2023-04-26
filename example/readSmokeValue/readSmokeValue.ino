/*****************************************************************
File:         readSmokeValue     
              Description:  Obtain the data package automatically output by the module every 8s, 
              and print the smoke detection value of channel A and channel B in the data package
******************************************************************/ 
#include "BM22S2021-1.h" 
uint8_t STATUS=5;
uint16_t smokeA;
uint16_t smokeB;
uint8_t Data[41]; 
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
}

void loop() { 
    if(SMOKE.isInfoAvailable()==1)
    {  
    SMOKE.readInfoPackage(Data); // Scaning the serial port received buffer to receive the information sent by the module
    printInfo();  // Print Smoke detection value  
    }     
}
void printInfo()
{
  /*Print Smoke detection value of channel A*/
  smokeA=(Data[17]<<8 | Data[16]);
  Serial.print("The current smoke detection value of channel A is "); 
  Serial.println(smokeA,DEC);

  /*Print Smoke detection value of channel B*/
  smokeB=(Data[19]<<8 | Data[18]);
  Serial.print("The current smoke detection value of channel B is "); 
  Serial.println(smokeB,DEC);
  Serial.println();
}
