/*****************************************************************
File:             BM22S2021-1.h
Author:           BESTMODULES
Description:      Define classes and required variables
History：         
V1.0.1-- initial version；2022-11-15；Arduino IDE : v1.8.13
******************************************************************/

#ifndef  _BM22S2021_h_
#define  _BM22S2021_h_
#include <Arduino.h>
#include <SoftwareSerial.h>
#define UART_BAUD 9600
#define CHECK_OK        0
#define CHECK_ERROR     1
#define TIMEOUT_ERROR   2

class BM22S2021_1
 {
    public:
    BM22S2021_1(uint8_t statusPin,HardwareSerial*theSerial);
    BM22S2021_1(uint8_t statusPin,uint8_t rxPin, uint8_t txPin);
    void  begin();
    uint8_t getSTATUS();
    uint16_t getFWVer();
    uint8_t getProDate(uint8_t buff[]);
    uint8_t requestInfoPackage(uint8_t buff[]);
    bool isInfoAvailable();
    void readInfoPackage(uint8_t array[]); 
    uint8_t readRegister(uint8_t addr);
    uint8_t readRunningVariables(uint8_t addr);
    uint8_t getAutoTx();
    uint8_t getStatusPinActiveMode();   
    uint8_t getT0ATopLimit();
    uint8_t getT0ABottomLimit();
    uint8_t getT0BTopLimit();
    uint8_t getT0BBottomLimit();
    uint16_t getT0AThreshold();
    uint16_t getT0BThreshold();
    uint8_t getDetectCycle();     //Set the standby smoke detection cycle
    uint8_t calibrateModule();
    uint8_t resetModule();
    uint8_t restoreDefault();
    uint8_t writeRegister(uint8_t addr,uint8_t data);
    uint8_t setAutoTx(uint8_t mode);
    uint8_t setStatusPinActiveMode(uint8_t state);  
    uint8_t setT0ACalibrateRange(uint8_t toplimit,uint8_t bottomlimit);
    uint8_t setT0BCalibrateRange(uint8_t toplimit,uint8_t bottomlimit);
    uint8_t setT0AThreshold(uint16_t value);
    uint8_t setT0BThreshold(uint16_t value);
    uint8_t setDetectCycle(uint8_t Cycle);
    
    private:
    uint8_t readBytes(uint8_t rbuf[], uint8_t rlen,uint16_t waitTime);
    void wirteBytes(uint8_t rbuf[], uint8_t rlen);
    void clear_UART_FIFO();
    uint8_t autoTxMode=0x80;
    uint8_t _rxPin,_txPin,_statusPin;
    uint8_t _recBuf[41] = {0}; // Array for storing received data
    HardwareSerial*_serial =NULL;
    SoftwareSerial*_softSerial =NULL;
 };
#endif
