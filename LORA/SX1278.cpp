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
/*Library for LoRa shield with SX1278. Base library with SX1278 functions.
* Shield and SX1278 CI uses SPI bus. So this library uses SPI library for 
* SX1278 commands and data sending/receiving.
* Look out! For this reason Arduino pins 13(sck),12(miso),11(mosi) are used by
* this shield.
* In addition, pin for CS (chip select) is used. But this pin can by selected
* between 10 and 8. But this library uses pin 10 by default. Change define in
* SX1278.h file if you decide to use pin 8.
* Another Arduino pin is used for reset: 5 or 7. Library uses 5 by default. 
* Change define in SX1278.h file if you decide to use pin 7. 
* Not all functionalities are interpreted by this library, but, in any case, 
* basic functions to display or set avery register are provided.
* 
* Author: Daniele Denaro 2015
*/

#include <SX1278.h>
//#include <AES.h>
static const float lorabwf[10]={7.8,10.4,15.6,20.8,31.25,41.7,62.5,125,250,500};
/******************************** General ************************************/

/* Initialize SPI and reset SX1278. 
   It returns false if shield is not available */
bool SX1278::begin()
{
  digitalWrite(SX1278Reset,1);
  pinMode(SX1278Reset,OUTPUT);
  initSPI();
  if (SPIread(RegVersion)==0) return false;
  restart();
  setState(STDBY);
//  SPIwrite(0x0B,0x2F); //Current trim : 120mA
//  SPIwrite(0x0B,0x32); //Current trim : 150mA
  SPIwrite(0x0B,0x35); //Current trim : 180mA
//  SPIwrite(0x0B,0x37); //Current trim : 200mA
//  setBoost(1);         //High output enabled
  setPower(2);         //default power 10dBm
  return true;
}

/* Reset */
void SX1278::restart()
{
  digitalWrite(SX1278Reset,0);
  delay(10);
  digitalWrite(SX1278Reset,1);
  delay(20);
}

/* Set operative state: 
   if FSKOOK: 0=SLEEP 1=STBY 2=FSTx 3=Tx 4=FSRx 5=Rx 
   if LORA  : 0=SLEEP 1=STBY 2=FSTx 3=Tx 4=FSRx 5=RxContinuous 6=RxSingle 7=CAD
*/
void SX1278::setState(byte opstate)
{
  byte b=SPIread(RegOpMode);
  b=setBit(b,opstate,0,3);
  SPIwrite(RegOpMode,b);
}

int SX1278::readState()
{
   unsigned char b=SPIread(RegOpMode);
   return b&B111;
}

/* Start SX1278 in LORA way */
void SX1278::startModeLORA()
{
  setState(SLEEP); //sleep mode
  byte b=SPIread(RegOpMode);
  bitSet(b,7);
  SPIwrite(RegOpMode,b);
}

/* Start SX1278 in standard way (FSK or OOK) */
void SX1278::startModeFSKOOK()
{
  setState(SLEEP); //sleep mode
  byte b=SPIread(RegOpMode);
  bitClear(b,7);
  SPIwrite(RegOpMode,b); 
}

/* 0:FSKOOK  1:LORA */
int SX1278::readMode()
{
  unsigned char b=SPIread(RegOpMode);
  return bitRead(b,7);
}

/* Set frequence in Mhz (SX12878 : 137-525 Mhz) (ex.: 433.92) (def.: 434.0)*/
void SX1278::setFreq(float freq) 
{
  union Bfreq
  {
    unsigned long ifreq;
    unsigned char vfreq[4];
  } bf;
  bf.ifreq=(freq*1000000)/61.035;
  SPIwrite(RegFrfMsb,bf.vfreq[2]); 
  SPIwrite(RegFrfMid,bf.vfreq[1]); 
  SPIwrite(RegFrfLsb,bf.vfreq[0]); 
}

float SX1278::readFreq()
{
  unsigned char f0=SPIread(RegFrfLsb);
  unsigned char f1=SPIread(RegFrfMid);
  unsigned char f2=SPIread(RegFrfMsb);
  unsigned long lfreq=f2;lfreq=(lfreq<<8)+f1;lfreq=(lfreq<<8)+f0;
  float freq=((float)lfreq*61.035)/1000000;
  return freq;
}

/* Set/get transmit power; pw can be:
*   1 (7dBm=5mW), 2 (10dBm=10mW), 3 (13dBm=20mW), 4 (17dBm=50mW)(def.), 5 (20dBm=100mW)
*   NB. It uses pin PA_BOOST because is the only possibility with lora module
*   (bit 7 of 0x09 register)
*/ 
void SX1278::setPower(byte pw)
{
  byte b1=0;
  byte b2=SPIread(0x4D);
  switch (pw)
  {
    case 1: b1=0xC5; b2=setBit(b2,4,0,3); SPIwrite(0x4D,b2);SPIwrite(0x09,b1);break;
    case 2: b1=0xC8; b2=setBit(b2,4,0,3); SPIwrite(0x4D,b2);SPIwrite(0x09,b1);break;
    case 3: b1=0xCB; b2=setBit(b2,4,0,3); SPIwrite(0x4D,b2);SPIwrite(0x09,b1);break;
    case 4: b1=0xCF; b2=setBit(b2,4,0,3); SPIwrite(0x4D,b2);SPIwrite(0x09,b1);break;
    case 5: b1=0xCF; b2=setBit(b2,7,0,3); SPIwrite(0x4D,b2);SPIwrite(0x09,b1);break;   
  }
}

/* Set low transmit power from 2 dBm to 6 dBm (from 1.25 mW to 4 mW)
*  db must be a integer value from 2 to 6 
*  NB. It uses pin PA_BOOST because is the only possibility with lora module 
*/
void SX1278::setLowPower(byte db)
{
  byte b1=0;
  byte b2=setBit(b2,4,0,3); SPIwrite(0x4D,b2);
  if (db<2) db=2;
  if (db>6) db=6;
  b1=0xC0+db-1;
  SPIwrite(0x09,b1); 
}

/* Format: 0 means code, 1 means dBm, 2 means mW */
float SX1278::getPower(byte format)
{
  if (format>2) format=2;
  float f[11][3]={{0,0,0},{0.2,2,1.5},{0.3,3,2},{0.4,4,2.5},{0.5,5,3.1},{0.6,6,4},
               {1,7,5},{2,10,10},{3,13,20},{4,17,50},{5,20,100}};
  byte b1=getBit(SPIread(0x09),0,4);
  byte b2=getBit(SPIread(0x4D),0,3);
  if (b1==1) return f[1][format];
  if (b1==2) return f[2][format];
  if (b1==3) return f[3][format];
  if (b1==4) return f[4][format];
  if (b1==5) return f[5][format];
  if ((b1>5)&(b1<=8)) return f[6][format];
  if ((b1>8)&(b1<=11)) return f[7][format];
  if ((b1>11)&(b1<15)) return f[8][format];
  if ((b1==15)&(b2==4)) return f[9][format];
  if ((b1==15)&(b2==7)) return f[10][format];
  return f[0][format];
}

/* Read version format: Mj.mi */
char* SX1278::readVersion(char sver[4])
{
  int ver=SPIread(RegVersion);
  byte ver0=ver&0xF;
  byte ver1=ver>>4;
  snprintf(sver,4,"%1u.%1u",ver1,ver0);
  return sver;
}

/* Set io pin function (DI0 to DI5) */ 
void SX1278::setIOpin(byte nio,byte val)
{
  byte b0=SPIread(0x40);
  byte b1=SPIread(0x41);
  byte b;
  switch (nio)
  {
    case 0: b=setBit(b0,val,6,2);SPIwrite(0x40,b);break;
    case 1: b=setBit(b0,val,4,2);SPIwrite(0x40,b);break;
    case 2: b=setBit(b0,val,2,2);SPIwrite(0x40,b);break;
    case 3: b=setBit(b0,val,0,2);SPIwrite(0x40,b);break;
    case 4: b=setBit(b1,val,6,2);SPIwrite(0x41,b);break;
    case 5: b=setBit(b1,val,4,2);SPIwrite(0x41,b);break;
  }
}

int SX1278::readTemp()
{
  char b=SPIread(0x3C);
  int t=int(b);
  return -t;
}

void SX1278::tempCalib()
{
  setState(STDBY);
  delayMicroseconds(100);
  setState(FSRX);
  byte b=SPIread(0x3b);
  bitClear(b,0);
  SPIwrite(0x3b,b);
  delayMicroseconds(150);
  bitSet(b,0);
  SPIwrite(0x3b,b);
  setState(STDBY);
}

/***************************** FSK/OOK mode **********************************/

/* When in standard way, this function decides if FSK or OOK modulation */
void SX1278::setModulation(byte mod) //0=FSK 1=OOK
{
  unsigned char b=SPIread(RegOpMode);
  b=setBit(b,mod,5,2);
  SPIwrite(RegOpMode,b|mod);
}

int SX1278::readModulation()
{
  unsigned char b=SPIread(RegOpMode);
  return bitRead(b,5);
}

/* Set bit rate (BPS) (mode FSKOOK) (def.: 4800)*/
void SX1278::setBPS(int bps)
{
  unsigned long lrate=32000000/bps;
  unsigned int rate=lrate;
  SPIwrite(RegBitrateMsb,highByte(rate));
  SPIwrite(RegBitrateLsb,lowByte(rate));
}

unsigned int SX1278::readBPS()
{
  unsigned char b0=SPIread(RegBitrateLsb);
  unsigned char b1=SPIread(RegBitrateMsb);
  unsigned int bps=word(b1,b0);
  unsigned long lbps=32000000/bps;
  bps=lbps;
  return bps;
}

/* Set/reset Packet mode */   
void SX1278::setPackNoPack(byte yesno)
{
     byte b=SPIread(0x31);
     if (yesno==1) bitSet(b,6); else bitClear(b,6);
     SPIwrite(0x31,b); 
}

void SX1278::setFIFOthr(byte nb)
{
  if (nb<1) nb=0;else nb=nb-1;
  byte b=SPIread(0x35);
  b=setBit(b,nb,0,6);
  SPIwrite(0x35,b);
}

byte SX1278::readFIFOthr()
{
  byte b=SPIread(0x35);
  b=0x3F&b;
  return b+1;
}

/* set preamble length (mode FSKOOK) (def.:3) (preamble byte type=0|1 0=0xAA, 1=0x55) */
void SX1278::setPreamble(unsigned int len, byte type)
{
  byte len0=lowByte(len);
  byte len1=highByte(len);
  SPIwrite(0x25,len1);
  SPIwrite(0x26,len0);
  byte b=SPIread(0x27);
  if (type==0) b=bitClear(b,5); else b=bitSet(b,5);
  SPIwrite(0x27,b);
  b=SPIread(0x1F);
  if (len==0) b=bitClear(b,7); else b=bitSet(b,7);
  SPIwrite(0x1F,b);
}

/* Set sync bytes: on/off (def. on), sync bytes (def: 3=0x01,0x01,0x01), val can be null if len=0*/
void SX1278::setSync(byte on,byte val[],byte len)
{
  byte b=SPIread(0x27);
  b=setBit(b,len,0,3);
  b=bitWrite(b,4,on);
  SPIwrite(0x27,b);
  if (len==0) return;
  int i;
  if (len>8) len=8;
  for (i=0;i<len;i++) SPIwrite(0x28+i,val[i]);
}

/* Set packet format: fix/var 0/1(def.: fix), crc/nocrc 0/1(def.: crc), payload length (def.: 1(min)) */
void SX1278::setPacket(byte variable,byte crc,byte len)
{
  byte b=SPIread(0x30);
  b=bitWrite(b,7,variable);
  b=bitWrite(b,4,crc);
  SPIwrite(0x30,b);
  SPIwrite(0x32,len);
}

/*** Transmission/reception ***/


/* Load FIFO with data to transmit (mode FSKOOK)*/
void SX1278::dataToSend(byte data[],int len)
{
  int i;
  for (i=0;i<len;i++) { SPIwrite(0x0,data[i]);}
}

/* Get register flags 1 or 2 */
byte SX1278::getFlags(byte nFreg)
{
  if (nFreg==1) return SPIread(0x3E);
  else return SPIread(0x3F);
}

/* Get single flag n of flag register 1 or 2 */
byte SX1278::getFlag(byte nFreg, byte nFlag)
{
  byte reg;
  if (nFreg==1) reg=0x3E; else reg=0x3F;
  return bitRead(SPIread(reg),nFlag);
}

void SX1278::resetFlag(byte nFreg, byte nFlag)
{
  byte reg;
  if (nFreg==1) reg=0x3E; else reg=0x3F;
  byte b=SPIread(reg);
  if (bitRead(b,nFlag)==1) b=bitSet(b,nFlag);
  SPIwrite(reg,b);  
}

byte SX1278::dataAvailable()
{
  if (getFlag(2,2)==1){return SPIread(0x32);} 
  else return 0;
}

byte SX1278::dataReceived(byte data[],int len)
{
  int i;
  for (i=0;i<len;i++) { data[i]=SPIread(0x0); if (bitRead(SPIread(0x3f),6)) return i;}
  return len;
}

void SX1278::fifoDiscard()
{
  int i;
  for (i=0;i<66;i++) {if (bitRead(SPIread(0x3f),6)==0) SPIread(0x0); else break;} 
}

/* Set sample number for average; val is power of two +1 (Ex. 0->2, 1->4, 2->8 ... 7->256) */ 
void SX1278::setRSSIsample(byte val)
{
  byte b=SPIread(0x0E);
  b=setBit(b,val,0,3);
  SPIwrite(0x0E,b);
}

float SX1278::readRSSIval()
{
  byte b=SPIread(0x11);
  float v=b;
  return -v/2;
}

void SX1278::setRSSIthr(int dBm)
{
  if (dBm<0) dBm=-dBm;
  SPIwrite(0x10,dBm*2);  
}

float SX1278::readRSSIthr()
{
  byte b=SPIread(0x10);
  float v=b;
  return -v/2; 
}

/* OOK threshold mode: ty=0 fixed, ty=1 peak (def), ty=2 average */
void SX1278::setOOKthrMode(byte ty)
{
  if (ty>2) ty=2;
  byte b=SPIread(0x14);
  b=setBit(b,ty,3,2);
  SPIwrite(0x14,b);
}

void SX1278::setBitSync(byte yesno)
{
  byte b=SPIread(0x14);
  b=bitWrite(b,5,yesno);
  SPIwrite(0x14,b);
}
/* OOK threshold trimming: val=0-:-7 dB=0.5-:-6 */
void SX1278::setOOKThrStep(byte val)
{
  byte b=SPIread(0x14);
  b=setBit(b,val,0,3);
  SPIwrite(0x14,b);
}

void SX1278::setOOKThrDec(byte val)
{
  byte b=SPIread(0x16);
  b=setBit(b,val,5,3);
  SPIwrite(0x16,b); 
}

void SX1278::setOOKbaseThr(int thr)
{
  if (thr<0) thr=-thr;
  byte b=byte(thr*2);
  SPIwrite(0x15,b);
}

float SX1278::readOOKbaseThr()
{
  byte b=SPIread(0x15);
  float v=-b;
  return v/2;
}

void SX1278::preambleDetect(byte yesno)
{
  byte b=SPIread(0x1F);
  b=bitWrite(b,7,yesno);
  SPIwrite(0x1F,b);  
}



/******************************* LoRa mode ***********************************/

/* Get LORA flags: 
  RxTimeout 7, RxDone 6, PayloadCrcError 5, ValidHeader 4, 
  TxDone 3, CadDone 2, FhssChangeChannel 1, CadDetected 0
*/
bool SX1278::getLoraFlag(byte flag)
{
  byte b=SPIread(0x12);
  return bitRead(b,flag);
}

/* reset LORA flag */
void SX1278::clearLoraFlag(byte flag)
{
  byte b=SPIread(0x12);
  byte mask=0; bitSet(mask,flag);
  SPIwrite(0x12,b&mask);
}

/* reset all LORA flags */
void SX1278::clearAllLoraFlag()
{
  byte b=SPIread(0x12);
  b=b&0xFF;
  SPIwrite(0x12,b);
}

/* get LORA flags for timeout or rx done 
   It returns 0 if nothing, 1 if packet received, -1 if timeout */
int SX1278::getLoraRxEndFlag()
{
  byte b=SPIread(0x12);
  if (bitRead(b,6)) return 1; 
  if (bitRead(b,7)) return -1;
  return 0;
}

/* load FIFO whith data to send (LORA mode) */
void SX1278::setLoraDataToSend(byte data[],byte datalen)
{
  if (readMode()!=loramode) return;
  byte baseadd=SPIread(0x0E);
  SPIwrite(0x0d,baseadd);
  int i;
  for (i=0;i<datalen;i++) SPIwrite(0,data[i]);
  SPIwrite(0x22,datalen);
}

/* Set Spreading Factor code. Spr.Factor values: 6,7,8,9,10,11,12 (def.: 7)*/
void SX1278::setLoraSprFactor(byte spf)
{
  byte b=SPIread(0x1E);
  if (spf<6) spf=6;
  if (spf>12) spf=12;
  b=setBit(b,spf,4,4);
  SPIwrite(0x1E,b);
}

byte SX1278::getLoraSprFactor()
{
  byte b=SPIread(0x1E);
  byte val=getBit(b,4,4);
  return val;
}

/* Set Banwidth (from 7.8 kHz to 500 kHz). Code: from 0 to 9 (def.: 7=125 kHz)*/
void SX1278::setLoraBw(byte bw)
{
  byte b=SPIread(0x1D);
  if (bw>9) bw=9;
  b=setBit(b,bw,4,4);
  SPIwrite(0x1D,b);
}

/* Get Lora Bw in terms of code */
byte SX1278::getLoraBw()
{
  byte b=SPIread(0x1D);
  byte val=getBit(b,4,4);
  return val;
}

/* Get Lora Bw in terms of frequency interval in kHz*/
float SX1278::getLoraBwFreq()
{
  byte i=getLoraBw();
  return lorabwf[i];
}

/* Set Coding Rate. Values 1=4/5, 2=4/6, 3=4/7, 4=4/8 (def.: 1) */
void SX1278::setLoraCr(byte cr)
{
  byte b=SPIread(0x1D);
  if (cr<1) cr=1;
  if (cr>4) cr=4;
  b=setBit(b,cr,1,3);
  SPIwrite(0x1D,b);
}

byte SX1278::getLoraCr()
{
  byte b=SPIread(0x1D);
  byte val=getBit(b,1,3);
  return val;
}

/* Symbol rate computation */
float SX1278::getSRate()
{
//  byte bw=SPIread(0x1D)>>4;
  byte sf=SPIread(0x1E)>>4;
//  float fbw=(pow(2,bw)+8)*1000;
  float fbw=getLoraBwFreq()*1000;
  float fsf=pow(2,sf);
  return fbw/fsf;
}

/* Bit per second */
float SX1278::getLorabps()
{
   float srate=getSRate();
   float crate=4.0/(4.0+getLoraCr());
   int sf=getLoraSprFactor();
   return sf*crate*srate;
}

/* Set on in case of simbol rate < 62/sec (or bps < 1200) */
void SX1278::setLoraLowDataRateOptimize(boolean on)
{  
   setRegBit(0x26,3,on);
}

/* Set timeout in terms of symbols (bytes) (def 100) (max 1023). 
   So timeout in terms of milliseconds depends on Symbols Rate */
void SX1278::setLoraRxByteTout(int nbyte)
{
  if (nbyte<1) nbyte=1;
  if (nbyte>1023) nbyte=1023;
  SPIwrite(0x1F,lowByte(nbyte));
  if (nbyte>255) 
  {
    byte hb=highByte(nbyte);
    byte b=SPIread(0x1E); 
    bitWrite(b,0,bitRead(hb,0));bitWrite(b,1,bitRead(hb,1));
    SPIwrite(0x1E,b);
  }
}

/* Set timeout for data receiving (LORA mode) (def.: 100 symbols) 
   rough conversion is made. So, timeout can be espressed in seconds.milliseconds
   (def.: 0.1Sec) (max about from 200 milliseconds to 2000 (depending on SR)*/
void SX1278::setLoraRxTimeout(float sec)
{
//  float bps=getLorabps();
//  unsigned int tout=int(bps*sec);
  float Rs=getSRate();
  unsigned int tout=int(Rs*sec);
  setLoraRxByteTout(tout);
}

/* In bytes (symbols) */
int SX1278::getLoraRxByteTout()
{
  byte tout=SPIread(0x1F);
  byte tout2=SPIread(0x1E)&0x03;
  unsigned int wt=word(tout2,tout);
  return wt;
}

/* In seconds.milliseconds */
float SX1278::getLoraRxTimeout()
{
  int wt=getLoraRxByteTout();
  float bps=getLorabps();
  float tm=(float)wt/bps;
//  float rs=getSRate();
//  float tm=wt;tm=tm/rs;
  return tm;
}

/* set max bytes to receive (def.: 255) */
void SX1278::setLoraMaxRxBuff(byte len)
{
  SPIwrite(0x23,len);
}

/* read received bytes into buff */
int SX1278::readLoraData(byte buff[], byte blen)
{
  int len=0;
  byte startadd=SPIread(0x10);
  SPIwrite(0x0d,startadd); 
  byte n=SPIread(0x13);
  if (blen<n) len=blen; else len=n;
  int i;
  for(i=0;i<len;i++) buff[i]=SPIread(0);
  int c=n-blen;
  if (c>0) for(i=0;i<c;i++) SPIread(0);
  return len;
}

/* discard received bytes */
void SX1278::discardLoraRx()
{
  byte startadd=SPIread(0x10);
  SPIwrite(0x0d,startadd);  
  byte n=SPIread(0x13);
  int i;
  SPIwrite(0x0D,0);
  for(i=0;i<n;i++) SPIread(0);
}

int SX1278::lastLoraPacketRssi()      //dBm
{
  return -164+SPIread(0x1A);
}

int SX1278::lastLoraPacketSnr()        //dBm
{
  char snr=SPIread(0x19);
  int isnr=int(snr)/4;
  return isnr;
}

int SX1278::getLoraRssi()              //dBm
{
  return -164+SPIread(0x1B);
}

int SX1278::lastLoraPacketSignalPower()    //dBm
{
  int PRssi=lastLoraPacketRssi();
  int PSnr=lastLoraPacketSnr();
  return PRssi+PSnr;
}

/* Set Preamble length.Value: from 4 to 65535 (def. 12) */
void SX1278::setLoraPreambleLen(unsigned int len)
{
  byte bL=lowByte(len);
  byte bH=highByte(len);
  SPIwrite(0x21,bL);
  SPIwrite(0x20,bH);
}

unsigned int SX1278::getLoraPreambleLen()
{
   byte bL=SPIread(0x21);
   byte bH=SPIread(0x20);
   unsigned int v=word(bH,bL);
   return v;
}

/* Set on/off automatic payload CRC computation/detection  (def.: off)*/
void SX1278::setLoraCrc(byte yesno)
{
  byte b=SPIread(0x1E);
  bitWrite(b,2,yesno);
  SPIwrite(0x1E,b);
}

/* Set random seed using wideband RSSI noisy measuring */
unsigned int SX1278::setRndSeed()
{
  byte b1=SPIread(0x2C);
  byte b2=SPIread(0x2C);
  unsigned int r=word(b1,b2)+(!word(b2,b1));
  randomSeed(r);
  return r;
}


/************************** Utilities (basic functions) ***********************/

/* read and write SX1278 register */
int SX1278::SPIwrite(byte address,byte val)
{

  digitalWrite(ss,0);
  SPI.transfer(address | 0x80);
  delayMicroseconds(100);
  SPI.transfer(val);
  digitalWrite(ss,1); 
}

int SX1278::SPIread(byte address)
{
  digitalWrite(ss,0);
  SPI.transfer(address);
  delayMicroseconds(100);
  int val=SPI.transfer(0x00);
  digitalWrite(ss,1); 
  return val;
}

/* get and set single bit of register */
void SX1278::setRegBit(byte reg,byte n,byte onoff)
{
  byte b=SPIread(reg);
  b=bitWrite(b,n,onoff);
  SPIwrite(reg,b);
}

byte SX1278::getRegBit(byte reg,byte n)
{
  byte b=SPIread(reg);
  return bitRead(b,n);
}

/* read and set group of register bits */
void SX1278::setRegBits(byte reg,byte val,byte bst, byte len)
{
  byte b=SPIread(reg);
  b=setBit(b,val,bst,len);
  SPIwrite(reg,b);
}

byte SX1278::getRegBits(byte reg,byte bst, byte len)
{
  byte b=SPIread(reg);
  return getBit(b,bst,len);
}

/* Return a string rapresentation of register content. 
   Format: address(hex) -> bin-rappresentation hex-rapresentation */
char* SX1278::readRegBin(byte reg)
{
  byte b=SPIread(reg);
  int i;
  snprintf(RegBin,8,"%02X -> ",reg);
  for(i=0;i<8;i++) {RegBin[i+6]=0x30+bitRead(b,7-i);}
  snprintf(&RegBin[14],4," %02X",b);
  return RegBin;
}

/* Private functions: used by other functions */

/* Set radio output to Boost pin ouptut (DRF1278F has just this output; so it must be set */
void SX1278::setBoost(byte yesno)
{
  byte b=SPIread(0x09);
  b=bitWrite(b,7,yesno);
  SPIwrite(0x09,b);
}

bool SX1278::initSPI()
{
  digitalWrite(ss,1);
  pinMode(ss,OUTPUT);  
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST); 
//  SPI.setClockDivider(SPI_CLOCK_DIV16);
  digitalWrite(SX1278Reset,0);
  delay(1);
  digitalWrite(SX1278Reset,1);
}



byte SX1278::setBit(byte b,byte val,byte bst, byte len)
{
  int i;
  for (i=0;i<len;i++) bitWrite(b,i+bst,bitRead(val,i));
  return b; 
}

byte SX1278::getBit(byte b,byte bst, byte len)
{
  int i;
  byte val=0;
  for (i=0;i<len;i++) bitWrite(val,i,bitRead(b,i+bst));
  return val; 
}

/*************************** AES256 encryption *******************************/

/* Define a 32 bytes key using an integer value (0-65535). 
   The 32 bytes key will be used by encrypt and decrypt functions */
void SX1278::createKey(unsigned int keyval)
{
  randomSeed(keyval);
  Cr.set_key(32);
}

/* Encrypt buff (replace each byte) and return the same buffer. 
   Buffer length must be multiple of 16 bytes. The parameter nbk is the number
   of 16 bytes blocks. (I.E. nbk=bufferlen/16)
   This function uses a predefined 32 bytes key */ 
byte* SX1278::encryptBuff(byte *buff, int nbk)
{
  if (Cr.encrypt_buff(buff,nbk)==-1) return NULL;
  return buff;
}

/* Decrypt buff (replace each byte) and return the same buffer.
   Buffer length must be multiple of 16 bytes. The parameter nbk is the number
   of 16 bytes blocks. (I.E. nbk=bufferlen/16)
   This function uses a predefined 32 bytes key */ 
byte* SX1278::decryptBuff(byte *buff,int nbk) 
{
  if (Cr.decrypt_buff(buff,nbk)==-1) return NULL;
  return buff;
}

byte* SX1278::getKey(){return Cr.key_sched;}

