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

#include <REMOTEC.h>

SX1278 SX;

/* Initialize shield as radiocontrol (Avidsen or Velleman)*/
bool REMOTEC::begin()
{
  if (!SX.begin()) return false;
  delay(1);
  setModeRemoteC();
  return true;
}

/* Change mode to transmit commands if shield already started in different mode*/
void REMOTEC::setTransmitMode()
{
  setModeRemoteC();
}

/* Send command (on/off) to Avidsen socket 
   address byte value=0/1/2(open)  sock=1/2/3/4/5 on/off=1/0 */
void REMOTEC::avidsenSet(byte address[5], byte sock, byte onoff) // 
{
  int i;
  SX.setBPS(3800);  
  for (i=0;i<5;i++) pack[i]=setByte(address[i]);
  for (i=5;i<10;i++) if (i==sock+4) pack[i]=setByte(0); else pack[i]=setByte(2);
  if (onoff==1) {pack[10]=setByte(0);pack[11]=setByte(1);}
  else {pack[10]=setByte(1);pack[11]=setByte(0);}
  pack[12]=0x80;pack[13]=0;pack[14]=0;pack[15]=0;
  sendPack(pack);
  delay(200);  
  
//  Serial.print("Sent: ");
//  for (i=0;i<12;i++) {Serial.print(getVal(pack[i]));Serial.print(" ");} 
//  Serial.println(); 

}

/* Send command (on/off) to Velleman socket 
   address byte value=0/1/2(open)  sock=1/2/3 on/off=1/0 */
void REMOTEC::vellemanSet(byte address[7],byte sock, byte onoff)
{
  int i;
  SX.setBPS(9600);  
  for (i=0;i<7;i++) pack[i]=setByte(address[i]);
  pack[7]=setByte(2);
  if (sock==3) {pack[7]=setByte(1);pack[8]=setByte(0);pack[9]=setByte(0);}
  else{ pack[8]=setByte(bitRead(sock,1)); pack[9]=setByte(bitRead(sock,0));}
  if (onoff==1) {pack[10]=setByte(0);pack[11]=setByte(1);}
  else {pack[10]=setByte(1);pack[11]=setByte(0);}
  pack[12]=0x80;pack[13]=0;pack[14]=0;pack[15]=0;
  sendPack(pack);
  delay(200);   
  
//  Serial.print("Sent: ");
//  for (i=0;i<12;i++) {Serial.print(getVal(pack[i]));Serial.print(" ");} 
//  Serial.println(); 
  
}

/* Set SX1278 in "continuous" mode. I.E. no processing is performed and impulses
   can be received on pin 6/9.
   Impulses can be received and decoded by scanImpulses() function.
*/
boolean REMOTEC::setScannerMode()
{
  SX.setState(1);
  delay(1);
  SX.setPackNoPack(0);
//  SX.preambleDetect(0);
//  SX.setSync(0,NULL,0); 
  SX.setRSSIsample(1); 
  SX.setBitSync(0);
  SX.setOOKThrStep(0); 
  SX.setOOKThrDec(0);
  pinMode(pinp,INPUT);
  SX.setOOKthrMode(1);
//  SX.setOOKthrMode(0);
  SX.setOOKbaseThr(basePk);  
  SX.setState(4);
  delay(1);
  SX.setState(5);
  delay(10);
  if (SX.getFlag(1,6)==1) return true; 
  else return false;
}

/* Receive impulses. If nothing is arrived it returns 0. You can put this function in Arduino loop function.
   If impulses are coming they are showed and decoded.
*/
int REMOTEC::scanImpulses()
{
  int nb=rData();
  if (nb==0) return nb;
  showData(nb);
  decodeData(nb);
  return nb;
}


byte REMOTEC::setByte(byte v)
{
  switch (v)
  {
    case 0: return 0x88;
    case 1: return 0xEE;
    case 2: return 0x8E;  
  }
  return 0x8E;
}

byte REMOTEC::getVal(byte b)
{
    switch (b)
  {
    case 0x88: return 0;
    case 0xEE: return 1;
    case 0x8E: return 2;  
  }
  return 9;
}

void REMOTEC::sendPack(byte by[16])
{
  SX.setState(2);
  delay(1);
  SX.dataToSend(by,16);
  SX.dataToSend(by,16);
  SX.dataToSend(by,16);
  SX.dataToSend(by,16);
  SX.setState(3);
}

void REMOTEC::setModeRemoteC()
{
//  SX.setState(0);
  SX.startModeFSKOOK();
  SX.setModulation(1);  //set OOK
  SX.setFreq(433.92);
  SX.setIOpin(5,2);
  SX.setPreamble(0,0);
  SX.setSync(0,NULL,0);
  SX.setPacket(0,0,64);
}

int REMOTEC::rData()
{
  int cd;
  pulse=pulseIn(pinp,HIGH);
  if (pulse>0) 
  {
    data[0]=pulse;
    for (cd=1;cd<ldata;cd++)
      {pulse=pulseIn(pinp,HIGH,2500);if (pulse==0) return cd; else data[cd]=pulse;}
    return cd+1;
  }
  return 0;
}


void REMOTEC::showData(int len)
{
    int i;
    unsigned long pmed=0;
    unsigned int pmin=50000;
    for (i=0;i<len;i++){if (data[i]<pmin) pmin=data[i];}
    pthr=2*pmin;
    int n=0;
    for (i=0;i<len;i++){if (data[i]<pthr) {pmed=pmed+data[i];n++;}}
    pmed=pmed/n;pthr=pmed*2;
    Serial.println();
    Serial.print("Impulses received: ");Serial.print(len); 
    Serial.print("   Short impulse (mean): ");Serial.println(pmed); 
    for (i=0;i<len;i++)
    {if (i%26==0) Serial.println(); Serial.print(data[i]);Serial.print(" ");}
    Serial.println(); 
}

void REMOTEC::decodeData(int len)
{
    int i;
    len=int(len/2);len=len*2;
    for(i=0;i<len;i=i+2)
    { Serial.print(decodeP(data[i],data[i+1]));Serial.print(" ");}
    Serial.println();
}

int REMOTEC::decodeP(unsigned int pa, unsigned int pb)
{
    if (pa<pthr) {if (pb<pthr) return 0; else return 2;}
    else return 1;
}

