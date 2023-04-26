/*****************************************************************
  File:             BM22S2021-1cpp
  Author:           BESTMODULES
  Description:      Communication and operation function with module
  History：
  V1.0.1-- initial version；2022-11-15；Arduino IDE : v1.8.13
******************************************************************/
#include  "BM22S2021-1.h"

/**********************************************************
Description: Select the hardware serial port you need to use
Parameters:  *theSerial：hardware serial 
             BMduino optional:serial(default) serial1/seria2/seria3/seria4
             UNO optional:serial(default)
             statusPin：status pin
Return:      none    
Others:      
**********************************************************/
BM22S2021_1::BM22S2021_1(uint8_t statusPin,HardwareSerial*theSerial)
{
  _serial = theSerial;
  _softSerial = NULL;
  _statusPin = statusPin;
}
/**********************************************************
Description: Select the software serial port RX TX you need to use
Parameters:  statusPin：status pin
             rxPin:RX pin on the development board
             txPin:TX pin on the development board
Return:      none    
Others:      
**********************************************************/
BM22S2021_1::BM22S2021_1(uint8_t statusPin,uint8_t rxPin, uint8_t txPin)
{
  _serial = NULL;
  _statusPin = statusPin;
  _rxPin = rxPin;
  _txPin = txPin;
  _softSerial = new SoftwareSerial(_rxPin, _txPin);
}
/**********************************************************
Description: Set serial baud rate
Parameters:  uartBaud：9600(default)
Return:      none
Others:
**********************************************************/
void BM22S2021_1::begin()
{
  if (_softSerial != NULL)
  {
    _softSerial->begin(UART_BAUD);
  }
  else
  {
    _serial->begin(UART_BAUD);
  }
  pinMode(_statusPin,INPUT);
}
/**********************************************************
Description: Get STATUS pin level
Parameters: None
Return: None
Others: None
**********************************************************/
uint8_t BM22S2021_1::getSTATUS()
{
  return digitalRead(_statusPin);
}

/**********************************************************
Description: Query the FW version
             The FW version number and production date are both 8421 BCD code.
Parameters:  none
Return:      0:read error
             data:16bit FWVer
Others:
**********************************************************/

uint16_t BM22S2021_1::getFWVer()
{
  clear_UART_FIFO();
  uint8_t uniCmd[4] = {0xAD, 0x00, 0x00, 0x53};
  uint8_t ack[12];
  uint16_t FWVer=0;
  wirteBytes(uniCmd, 4);
  delay(50);
  if (readBytes(ack,12,10) == 0x00 && ack[4] == 0xAD)
  {
   FWVer=(ack[6]<<8 | ack[7]);
  }
  return   FWVer;
}
/**********************************************************
Description: Query production date.
             The FW version number and production date are both 8421 BCD code.
Parameters:  buff[]:3 byte
             buff[0]:year  buff[1]:month  buff[2]:day
Return:      1: module data acquisition failed, there is no correct feedback value
             0: Module data obtained successfully
Others:
**********************************************************/
uint8_t BM22S2021_1::getProDate(uint8_t buff[])
{
  clear_UART_FIFO();
  uint8_t uniCmd[4] = {0xAD, 0x00, 0x00, 0x53};
  uint8_t ack[12];
  wirteBytes(uniCmd, 4);
  delay(50);
  if (readBytes(ack,12,10) == 0x00 && ack[4] == 0xAD)
  {
    buff[0]=ack[8];
    buff[1]=ack[9];
    buff[2]=ack[10];
    return    0;
  }
  else
  {
    return   1;
  }
}

/**********************************************************
Description:  Read the current status and data of the equipment;
Parameters:  buff:Store the read data //buff[40-0]/buff[20-0]
Return: 0: Read data successfully, module feedback is correct
        1: failed to read data, module feedback error
Others:
**********************************************************/
uint8_t BM22S2021_1::requestInfoPackage(uint8_t buff[])
{
   clear_UART_FIFO();
  uint8_t uniCmd[4] = {0xAC, 0x00, 0x00, 0x54};
  wirteBytes(uniCmd, 4);
  delay(50);
  if (readBytes(buff,41,10) == 0x00 && buff[4] == 0xAC)
  {
    return    0;
  }
  else
  {
    return   1;
  }
}
/**********************************************************
Description: Determine whether it is the data packet sent by the module
Parameters:  none
Return:      1:Have 41/21 data bytes
             0:no data
Others:     
**********************************************************/
bool BM22S2021_1::isInfoAvailable()
{
  uint8_t recLen=41;
if(autoTxMode==0x80)
{
  recLen=41;
}
if(autoTxMode==0x81)
{
  recLen=21;
}
  uint8_t header[5] = {0xAA,recLen, 0x11, 0x01, 0xAC};
  uint8_t recBuf[recLen] = {0};
  uint8_t i, num = 0, readCnt = 0, failCnt = 0, checkCode = 0;
  bool isHeader = 0, result = false;

  /* Select hardSerial or softSerial according to the setting */
  if (_softSerial != NULL)
  {
    num = _softSerial->available();
  }
  else if (_serial != NULL)
  {
    num = _serial->available();
  }
  /* Serial buffer contains at least one 32-byte data */
  if (num >= recLen)
  {
    while (failCnt < 2) // Didn't read the required data twice, exiting the loop
    {
      /* Find 5-byte data header */
      for (i = 0; i < 5;)
      {
        if (_softSerial != NULL)
        {
          recBuf[i] = _softSerial->read();
        }
        else if (_serial != NULL)
        {
          recBuf[i] = _serial->read();
        }
        if (recBuf[i] == header[i])
        {
          isHeader = 1; // Fixed code is correct
          i++;             // Next byte
        }
        else if (recBuf[i] != header[i] && i > 0)
        {
          isHeader = 0; // Next fixed code error
          failCnt++;
          break;
        }
        else if (recBuf[i] != header[i] && i == 0)
        {
          readCnt++; // 0xAA not found, continue
        }
        if (readCnt >= (num - 5))
        {
          readCnt = 0;
          isHeader = 0; //
          break;
        }
      }
      /* Find the correct data header */
      if (isHeader)
      {
        for (i = 0; i < 5; i++)
        {
          checkCode += recBuf[i]; // Sum checkCode
        }
        for (i = 5; i < recLen; i++) // Read subsequent 27-byte data
        {
          if (_softSerial != NULL)
          {
            recBuf[i] = _softSerial->read();
          }
          else if (_serial != NULL)
          {
            recBuf[i] = _serial->read();
          }
          checkCode += recBuf[i]; // Sum checkCode
        }
        checkCode = checkCode - recBuf[recLen - 1];
        checkCode = (~checkCode) + 1; // Calculate checkCode
        /* Compare whether the check code is correct */
        if (checkCode == recBuf[recLen - 1])
        {
          for (i = 0; i < recLen; i++) // True, assign data to _recBuf[]
          {
            _recBuf[i] = recBuf[i];
          }
          result = true;
          break; // Exit "while (failCnt < 2)" loop
        }
        else
        {
          failCnt++; // Error, failCnt plus 1, return "while (failCnt < 2)" loop
          checkCode = 0;
        }
      }
    }
    clear_UART_FIFO();
  }
  return result;

}
/**********************************************************
Description: Read the data automatically output by the module
Parameters:   buff:41/21 byte
Return:    
Others:     
**********************************************************/
void BM22S2021_1::readInfoPackage(uint8_t array[])
{
  uint8_t recLen=41;
if(autoTxMode==0x80)
{
  recLen=41;
}
if(autoTxMode==0x81)
{
  recLen=21;
}
  for (uint8_t i = 0; i < recLen; i++)
  {
    array[i] = _recBuf[i];
  }
}
/**********************************************************
Description:  Read the register of the specified address
Parameters:  
        addr:Destination register address to read
        Register list:
        1.T0A upper / lower calibration limit (H+L):0X08  0X09
        2.T0B upper / lower calibration limit (H+L):0X0A  0X0B
        3.T0A alarm threshold(H+L):0X10 0X11
        4.T0B alarm threshold(H+L):0X12 0X13
        5. standby smoke detection cycle:0X2D
        6. serial port automatic output:0X2E
        7. alarm output level:0X2F
        data:Data to be written
        All available addresses have been defined in the H file      
Return: data:Store read data
        0:read error
Others:
**********************************************************/
uint8_t BM22S2021_1::readRegister(uint8_t addr)
{
   clear_UART_FIFO();
  uint8_t data=0;  
  uint8_t uniCmd[4] = {0xD0,addr,0x00,0x00};
  uint16_t num=0xD0 + addr +0x00;
  uniCmd[3] = ~(lowByte(num)) + 1;
  uint8_t uniAck[8];
  wirteBytes(uniCmd, 8);
  delay(50);
  if (readBytes(uniAck,8,10) == 0x00 && uniAck[4] == 0xD0)
  {
    data=uniAck[6];
  }
  return  data;
}
/**********************************************************
Description: Read the run variable at the specified address
Parameters:  addr:Run variable address
        All available addresses have been defined in the H file
        Run variable list:
        Equipment status :0X90
        VBG voltage a/d value(H+L) :0X91 0X92
        Reference value(H+L) :0X93 0X94
        T0A status (channel a) :0X95
        T0B status (channel B) :0X96
        T0B smoke detection value(H+L) :0X9b  0X9c
        T0B smoke detection value(H+L) :0X9D  0X9E
        T0A calibration zero(H+L) :0X9F 0XA0
        T0B calibration zero(H+L) :0XA1 0XA2
        T0A alarm threshold(H+L) :0XA3  0XA4
        T0B alarm threshold(H+L) :0XA5  0XA6
        A. Variation ratio of channel B :0XA7
        Alarm count :0XAA
        Temperature a/d value(H+L) :0XAB  0XAC
        Fault count :0XAF
        Smoke count :0XB0
        VDD voltage(H+L) :0XB1  0XB2       
Return: data:Store read data 
        0:read error
Others:
**********************************************************/
uint8_t BM22S2021_1::readRunningVariables(uint8_t addr)
{
   clear_UART_FIFO();
  uint8_t data=0;  
  uint8_t uniCmd[4] = {0xD2,addr,0x00,0x00};
  uint16_t num=0xD2 + addr + 0x00;
  uniCmd[3] = ~(lowByte(num)) + 1;
  uint8_t uniAck[8];
  wirteBytes(uniCmd, 4);
  delay(50);
  if (readBytes(uniAck,8,10) == 0x00 && uniAck[4] == 0xD2)
  {
    data=uniAck[6];
  }
  return  data;
}
/**********************************************************
Description: Get the current serial port automatic output status
Parameters:  none   
Return:      0x80 Detailed data is automatically output for each smoke detection
             0x81 only Simple data output
             0x00 does not automatically output data;
Others:
**********************************************************/
uint8_t BM22S2021_1::getAutoTx()
{
   clear_UART_FIFO();
  uint8_t mode=0;
  mode=readRegister(0x2e);
  return mode;
}
/**********************************************************
Description: Get the level that the STATUS pin will output under the current alarm state
Parameters:  none   
Return:      0x80 HIGH level
             0x00 LOW level
Others:
**********************************************************/
uint8_t BM22S2021_1::getStatusPinActiveMode()
{
   clear_UART_FIFO();
  uint8_t level=0;
  level=readRegister(0x2f);
  return level;
}
/**********************************************************
Description: get T0A calibration Top limits
Parameters:  none
             bottomlimit:Air calibration lower limit
Return:      toplimit:Air calibration upper limit
Others:
**********************************************************/
uint8_t BM22S2021_1::getT0ATopLimit()
{
   clear_UART_FIFO();
  uint8_t toplimit=0;
  toplimit=readRegister(0x08);
  return  toplimit;
}
/**********************************************************
Description: get T0A calibration Top limits
Parameters:  none
             
Return:      bottomlimit:Air calibration lower limit
Others:
**********************************************************/
uint8_t BM22S2021_1::getT0ABottomLimit()
{
   clear_UART_FIFO();
  uint8_t bottomlimit=0;
  bottomlimit=readRegister(0x09);
  return  bottomlimit;
}
/**********************************************************
Description: get T0B calibration Top limits
Parameters:  none
             bottomlimit:Air calibration lower limit
Return:      toplimit:Air calibration upper limit
Others:
**********************************************************/
uint8_t BM22S2021_1::getT0BTopLimit()
{
   clear_UART_FIFO();
  uint8_t toplimit=0;
  toplimit=readRegister(0x0A);
  return  toplimit;
}
/**********************************************************
Description: get T0B calibration Top limits
Parameters:  none
             
Return:      bottomlimit:Air calibration lower limit
Others:
**********************************************************/
uint8_t BM22S2021_1::getT0BBottomLimit()
{
   clear_UART_FIFO();
  uint8_t bottomlimit=0;
  bottomlimit=readRegister(0x0B);
  return  bottomlimit;
}
/**********************************************************
Description: get the alarm threshold of T0A channel
Parameters:  none   
Return:      Threshold:12bit valid data
Others:
**********************************************************/
uint16_t BM22S2021_1::getT0AThreshold()
{
   clear_UART_FIFO();
  uint16_t Threshold=0;
 _recBuf[0]=readRegister(0x10);
  _recBuf[1]=readRegister(0x11);
  Threshold=(_recBuf[1]<<8 | _recBuf[0]);
  return Threshold;
}
/**********************************************************
Description: get the alarm threshold of T0B channel
Parameters:  none   
Return:      Threshold:12bit valid data
Others:
**********************************************************/
uint16_t BM22S2021_1::getT0BThreshold()
{
   clear_UART_FIFO();
  uint16_t Threshold=0;
  _recBuf[0]=readRegister(0x12);
  _recBuf[1]=readRegister(0x13);
  Threshold=(_recBuf[1]<<8 | _recBuf[0]);
  return Threshold;
}
/**********************************************************
Description: Detection cycle of smoke-free environment equipment in standby, unit: s;
Parameters:  none   
Return:      cycle:time
Others:
**********************************************************/

uint8_t BM22S2021_1::getDetectCycle()
{
   clear_UART_FIFO();
  uint8_t cycle=0;
  cycle=readRegister(0x2d);
  return cycle;
}
/**********************************************************
Description:Trigger the air calibration function. 
            The calibration time is 8s. The data bits in the calibration process will return to the current calibration timing in real time.
            The calibration is completed  The data bit returns 0xa0, and the failure returns 0xf0
Parameters:  
Return: 0: module feedback is correct
        1: module feedback error
Others:
**********************************************************/
uint8_t BM22S2021_1::calibrateModule()
{
  clear_UART_FIFO();
  uint8_t uniCmd[4] = {0xAB, 0x00, 0x00, 0x55};
  uint8_t uniAck[8];
  uint8_t i;
  wirteBytes(uniCmd, 4);
  delay(50);
  for(uint8_t c=0;c<9;c++)
  {
    i=readBytes(uniAck,8,10);
    if(c<8)
    {
      delay(1000);
    }
  }
  if (i == 0x00 && uniAck[6]==0xa0)
  {
    return    0;
  }
  else
  {
    return   1;
  }
}
/**********************************************************
Description: Send instructions to reset the chip in the module
Parameters:  none
Return:      1: module data acquisition failed, there is no correct feedback value
             0: Module data obtained successfully
Others:     
**********************************************************/
uint8_t BM22S2021_1::resetModule()
{
  clear_UART_FIFO();
  uint8_t uniCmd[4] = {0xAF, 0x00, 0x00, 0x51};
  uint8_t uniAck[8];
  wirteBytes(uniCmd, 4);
  delay(50);
  if (readBytes(uniAck,8,10) == 0x00 && uniAck[4] == 0xAF)
  {
    return    0;
  }
  else
  {
    return   1;
  }
}

/**********************************************************
Description: Send command to restore the module to factory settings
             Factory settings:
             1. TOA and T0B
             Calibration upper limit: 0xc8
             Lower calibration limit: 0x19
             2.T0A  Alarm threshold(L[7:0]:0x5e H[15:8]:0x01)
             3.T0B  Alarm threshold(L[7:0]:0x96 H[15:8]:0x00)
             4.Standby smoke detection cycle :  0x08
             5.Serial port automatic output:  0x80
             6.Alarm output level:0x80 HIGH
Parameters:  none
Return:      1: Module setting failed without correct feedback value
             0: Module set successfully
Others:
**********************************************************/
uint8_t BM22S2021_1::restoreDefault()
{
  clear_UART_FIFO();
  uint8_t uniCmd[4] = {0xA0,0x00,0x00,0x60};
  uint8_t uniAck[8];
  wirteBytes(uniCmd, 4);
  delay(350);
  if (readBytes(uniAck,8,10) == 0x00 && uniAck[4] == 0xA0)
  {
    return  0;
  }
  else
  {
    return  1;
  }
}
/**********************************************************
Description:  Write data to the register of the specified address;
Parameters:  
        addr:Destination register address to Write
        All available addresses have been defined in the H file
        Register list:
        1.T0A upper / lower calibration limit (H+L):0X08  0X09
        2.T0B upper / lower calibration limit (H+L):0X0A  0X0B
        3.T0A alarm threshold(H+L):0X10 0X11
        4.T0B alarm threshold(H+L):0X12 0X13
        5. standby smoke detection cycle:0X2D
        6. serial port automatic output:0X2E
        7. alarm output level:0X2F
        data:Data to be written
Return: 0: Write data successfully, module feedback is correct
        1: module feedback error
Others:
**********************************************************/
uint8_t BM22S2021_1::writeRegister(uint8_t addr,uint8_t data)
{
  clear_UART_FIFO();
  uint8_t uniCmd[4] = {0xE0, addr, data,0x00};
  uint16_t num=0xE0 + addr + data;
  uniCmd[3] = ~(lowByte(num)) + 1;
  uint8_t uniAck[8];
  wirteBytes(uniCmd, 4);
  delay(50);
  if(addr==0x2e)
  {
    autoTxMode=data;
  }
if (readBytes(uniAck,8,10) == 0x00 && uniAck[4] == 0xE0)
  {
    return    0;
  }
  else
  {
    return   1;
  }
}
/**********************************************************
Description: Set the enable mode of TX pin automatic output
Parameters:  mode:0x81-20byte 0x80--41byte  0x00--0byte
Return:      0:Setting succeeded
             1:Setting failed
Others:
**********************************************************/
uint8_t BM22S2021_1::setAutoTx(uint8_t mode)
{
   clear_UART_FIFO();
    uint8_t sign=writeRegister(0x2e,mode);
    autoTxMode=mode;
    if(sign==0)
  {
    return    0;
  }
  else
  {
    return    1;
  }
}
/**********************************************************
Description: set the current serial port automatic output status
Parameters:  state:0x80--HIGH level 0x0--LOW level
Return:      0:Setting succeeded
             1:Setting failed
Others:
**********************************************************/
uint8_t BM22S2021_1::setStatusPinActiveMode(uint8_t state)
{
   clear_UART_FIFO();
    uint8_t sign=writeRegister(0x2f,state);
    if(sign==0)
  {
    return    0;
  }
  else
  {
    return    1;
  }
}
/**********************************************************
Description: Set T0A calibration upper and lower limits
Parameters:  toplimit:Air calibration upper limit
             bottomlimit:Air calibration lower limit
Return:      0: set successfully
             1: Setting failed
Others:
**********************************************************/
uint8_t BM22S2021_1::setT0ACalibrateRange(uint8_t toplimit,uint8_t bottomlimit)
{
clear_UART_FIFO();
writeRegister(0x08,toplimit);
uint8_t sign=writeRegister(0x09,bottomlimit);
if(sign==0)
{
  return    0;
}
else
{
  return    1;
}
}
/**********************************************************
Description: Set T0B calibration upper and lower limits
Parameters:  toplimit:Air calibration upper limit
             bottomlimit:Air calibration lower limit
Return:      0: set successfully
             1: Setting failed
Others:
**********************************************************/
uint8_t BM22S2021_1::setT0BCalibrateRange(uint8_t toplimit,uint8_t bottomlimit)
{
   clear_UART_FIFO();
writeRegister(0x0A,toplimit);
uint8_t sign=writeRegister(0x0B,bottomlimit);
if(sign==0)
{
  return    0;
}
else
{
  return    1;
}
}

/**********************************************************
Description: set the alarm threshold of T0A channel
Parameters:  value:16bit  default:0x015e
Return:      0:Setting succeeded
             1:Setting failed
Others:
**********************************************************/
uint8_t BM22S2021_1::setT0AThreshold(uint16_t value)
{
   clear_UART_FIFO();
  _recBuf[0]=lowByte(value);
  _recBuf[1]=highByte(value);
  writeRegister(0x10,_recBuf[0]);
  uint8_t sign=writeRegister(0x11,_recBuf[1]);
  if(sign==0)
  {
    return    0;
  }
  else
  {
    return    1;
  }
}
/**********************************************************
Description: set the alarm threshold of T0B channel
Parameters:  value:16bit  default:0x0096
Return:      0:Setting succeeded
             1:Setting failed
Others:
**********************************************************/
uint8_t BM22S2021_1::setT0BThreshold(uint16_t value)
{
   clear_UART_FIFO();
   
  _recBuf[0]=lowByte(value);
  _recBuf[1]=highByte(value);
  writeRegister(0x12,_recBuf[0]);
  uint8_t sign=writeRegister(0x13,_recBuf[1]);
  if(sign==0)
  {
    return    0;
  }
  else
  {
    return    1;
  }
}
/**********************************************************
Description: set detection cycle of smoke-free environment equipment in standby
Parameters:  Cycle:time  default:0x08  unit: s;
Return:      0:Setting succeeded
             1:Setting failed
Others:
**********************************************************/
uint8_t BM22S2021_1::setDetectCycle(uint8_t Cycle)
{
   clear_UART_FIFO();
  uint8_t sign=writeRegister(0x2d,Cycle);
    if(sign==0)
  {
    return    0;
  }
  else
  {
    return    1;
  }
}

/**********************************************************
Description: UART readBytes
Parameters:  rbuf:Variables for storing Data to be read
             len:Length of data plus command
Return:      0: check ok
        1: check error
        2: timeout error
Others:
**********************************************************/
uint8_t BM22S2021_1::readBytes(uint8_t rbuf[], uint8_t rlen,uint16_t waitTime)
{
  uint8_t i, delayCnt, checkCode;
  /* Select hardSerial or softSerial according to the setting,
     check if there is data in the UART Receive FIFO */
  if (_softSerial != NULL)
  {
    for (i = 0; i < rlen; i++)
    {
      delayCnt = 0;
      while (_softSerial->available() == 0)
      {
        if (delayCnt > waitTime)
        {
          return TIMEOUT_ERROR; // Timeout error
        }
        delay(1);
        delayCnt++;
      }
      rbuf[i] = _softSerial->read(); // Receive data
    }
  }
  else if (_serial != NULL)
  {
    for (i = 0; i < rlen; i++)
    {
      delayCnt = 0;
      while (_serial->available() == 0)
      {
        if (delayCnt > waitTime)
        {
          return 1; // Timeout error
        }
        delay(1);
        delayCnt++;
      }
      rbuf[i] = _serial->read(); // Receive data
    }
  }

  /* Calculate check code*/
  for (i = 0, checkCode = 0; i < (rlen - 1); i++)
  {
    checkCode += rbuf[i];
  }
  checkCode = ~checkCode + 1;
  /* Proofread check code*/
  if (checkCode == rbuf[rlen - 1])
  {
    return CHECK_OK; // Check correct
  }
  else
  {
    return CHECK_ERROR; // Check error
  }

}

/**********************************************************
Description: UART wirteBytes
             Automatically determine whether a hardware or software serial  is used
Parameters:  buff:Variables for storing Data to be read
             datalength:Length of data plus command
Return:      none
Others:
**********************************************************/
void  BM22S2021_1::wirteBytes(uint8_t wbuf[], uint8_t wlen)
{
  if (_softSerial != NULL)
  {
    while (_softSerial->available()>0)
    {
      _softSerial->read();
    }
    _softSerial->write(wbuf, wlen);
  }
  else
  {
    while (_serial->available()>0)
    {
      _serial->read();
    }
    _serial->write(wbuf, wlen);
  }
}
/**********************************************************
Description: eliminate buff data
Parameters:  none
Return:      none    
Others:      
**********************************************************/ 

void BM22S2021_1::clear_UART_FIFO()
{
    if (_softSerial != NULL)
  {
    while (_softSerial->available() > 0)
    {
      _softSerial->read();
    }
  }
  else
  {
    while (_serial->available() > 0)
    {
      _serial->read();
    }
  }
}
