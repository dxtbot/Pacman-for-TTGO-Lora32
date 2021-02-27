/*
  Copyright (c) 2015 Daniele Denaro.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*/

/******************************************************************************/
/* Class REMOTEC. Library for 220V sockets remote control. 
*  It semplifies class SX1278 in case of OOK transmission for remote power
*  control as Avidsen or Velleman systems.
*  Just use begin function and avidsenSet() or vellemanSet() to switch remote
*  power sockets.
*  Principal functions:
*  - begin() : initialize shield for OOK modulation and 433.92 MHz (def.Pwr=10dBm).
*    If shield is not here it returns false.
*  - avidsenSet() or vellemanSet() : switch ON/OFF socket at address provided 
*  - setScannerMode() if you want detect impulse from transmitter. In this case
*    you have to move jumper JDI02 on DATA position. Besides jumper JDI03 has
*    to be on pin 6 position. 
* So, two type of use: send command or scan impulses to detect base impulse
* width and decode. Sending command is absolutely reliable. 
* But receiving impulses is not, and it is just for detecting system base impulse
* and address decoding, because SX1278 is configured for packet system with 
* preamble for sincronization. So, as receiver, the library uses "continuous" 
* mode without any other SX1278 contribution.
*    
* Author: Daniele Denaro 2015
*/

#ifndef REMOTEC_h
#define REMOTEC_h

#include <SX1278.h>

//#define pinp 7
#define pinp 6
#define ldata 32
#define basePk -60


class REMOTEC
{
  public:
  
/* Initialize shield as radiocontrol (Avidsen or Velleman)*/  
  bool begin();
  
/* Ready to transmit commands */  
  void setTransmitMode(); 
  
/* Set SX1278 in "continuous" mode. I.E. no processing is performed and impulses can be received on pin 3
   Impulses can be received and decoded by scanImpulses() function.*/
  boolean setScannerMode();  
  
/* Send command (on/off) to Avidsen socket 
   address byte value=0/1/2(open)  sock=1/2/3/4/5 on/off=1/0 */  
  void avidsenSet(byte address[5],byte socket, byte onoff);
  
/* Send command (on/off) to Velleman socket 
   address byte value=0/1/2(open)  sock=1/2/3 on/off=1/0 */  
  void vellemanSet(byte address[7],byte socket, byte onoff);
  
/* Receive impulses. If nothing is arrived it returns 0. You can put this function in Arduino loop function.
   If impulses are coming they are showed and decoded.*/
  int scanImpulses();

  private:
  
  void setModeRemoteC();  
  byte setByte(byte v);
  byte getVal(byte b);
  void sendPack(byte pack[16]);
  int rData();
  void showData(int len);
  void decodeData(int len);
  int decodeP(unsigned int pa, unsigned int pb);  

  unsigned long pulse;
  byte pack[16];
  unsigned int data[ldata];
  unsigned long pthr;
}; 


#endif
