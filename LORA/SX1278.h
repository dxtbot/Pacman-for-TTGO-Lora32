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
* basic functions to display or set avery register, are provided.
* 
* Author: Daniele Denaro 2015
*/

/* Version 1.2 2017 */

#ifndef SX1278_h
#define SX1278_h

#include <Arduino.h>
#include <SPI.h>
#include <AES.h>

#define SX1278Reset 9 
#define ss 10 

#if defined (__AVR_ATmega32U4__) // telecomand
// #define ss 17 
// #define SX1278Reset 5    // pin for reset for telecomand
#else 
// #define ss 10            // pin for chip select
 //#define SX1278Reset 9    // pin for reset for new shield and Maduino
#endif 

//#define SX1278Reset 5    // pin for reset for old shield

/* important register addresses */

#define RegFifo        0x00
#define RegOpMode      0x01

#define RegVersion     0x42

#define RegBitrateMsb  0x02
#define RegBitrateLsb  0x03


#define RegFrfMsb      0x06
#define RegFrfMid      0x07
#define RegFrfLsb      0x08

#define RegVersion     0x42

/* Radio mode values */
#define loramode       0x01
#define fskookmode     0x00

/* Operative mode values */

#define SLEEP          0x00 // Lowest power. It allows change OOK/FSK to LoRa
#define STDBY          0x01 // Basic state
#define FSTX           0x02 // Prepare to transmit
#define TX             0x03 // Transmit what is in FIFO queue
#define FSRX           0x04 // Prepare to set in receive mode
#define RXCONT         0x05 // Receive mode continuous in LoRa configuration
#define RX             0x05 // Receive mode in OOK/FSK configuration
#define RXSING         0x06 // Receive mode single packet in LoRa configuration
#define CAD            0x07 // Channel activity detection

/* LORA flags */

#define RxTimeout         0x07
#define RxDone            0x06
#define PayloadCrcError   0x05
#define ValidHeader       0x04
#define TxDone            0x03
#define CadDone           0x02
#define FhssChangeChannel 0x01
#define CadDetected       0x00

/***/

class SX1278
{
  public:

/****************************** General *************************************/
  
/* Initialize SPI and reset SX1278
   It returns false if shield is not available */  
   bool begin();
   
   void restart();   //reset
   
/* Set/read operative state: 
   if FSKOOK: 0=SLEEP 1=STDBY 2=FSTX 3=Tx 4=FSRX 5=RX 
   if LORA  : 0=SLEEP 1=STDBY 2=FSTX 3=Tx 4=FSRX 5=RXCONT 6=RXSING 7=CAD */  
   void setState(unsigned char s); 
   int readState();  
   
/* Start SX1278 in LORA way */
   void startModeLORA();
/* Start SX1278 in standard way (FSK or OOK) */   
   void startModeFSKOOK();
/* 0:FSKOOK  1:LORA */
   int readMode();
   
/* Set frequency in Mhz (SX12878 : 137-525 Mhz) (ex.: 433.92) (def.: 434.0)*/
   void setFreq(float freq); //freq in Mhz (ex.: 433.92)
   float readFreq();   

/* Set/get transmit power; pw can be:
   1 (7dBm=5mW), 2 (10dBm=10mW), 3 (13dBm=20mW), 
   4 (17dBm=50mW) (def.), 5 (20dBm=100mW)*/   
   void setPower(byte pw);  
   
/* Set low transmit power from 2 dBm to 6 dBm (from 1.25 mW to 4 mW)
   db must be a integer value from 2 to 6  */
   void setLowPower(byte db);
         
/* Format: 0 means return code, 1 means return dBm, 2 means return mW */
   float getPower(byte format);
      
/* Read version format: Mj.mi */  
   char* readVersion(char sver[4]); 
   
/* Set io pin function (DI0 to DI5) 
   nio: 0 to 5  val: 0 to 3  (default: all 0)
   See SX1278 datasheet for details: 
   - see tab. 30 for OOK/FSK packet mode; 
   - see tab. 29 for OOK/FSK continuous mode;
   - see tab. 18 for LoRa mode */ 
   void setIOpin(byte nio,byte val);   
   
/* Temperature onboard sensor function */
   int readTemp();
   void tempCalib();      

/***************************** FSK/OOK mode **********************************/

/*** Communication specifications ***/
  
/* When in standard way, this function decides if FSK (0) or OOK (1) modulation */   
   void setModulation(unsigned char mod); 
   int readModulation();
   
/* Set bit rate (BPS) (mode FSKOOK) (def.: 4800)*/   
   void setBPS(int bps);
   unsigned int readBPS();
   
/* Set/reset (1/0) Packet mode */   
   void setPackNoPack(byte yesno);
   
/* set preamble length (mode FSKOOK) (def.:3) (def. preamble chars: 0xAA) */   
   void setPreamble(unsigned int len, byte type);
/* Set sync bytes (address):
   on/off (def. on), sync bytes (def: 3=0x01,0x01,0x01), val can be null if len=0*/
   void setSync(byte on,byte val[],byte len);
/* Set packet format: 
   fix/var 0/1(def.: fix), nocrc/crc 0/1(def.: crc), payload length (def.: 1(min)) */   
   void setPacket(byte variable,byte crc,byte len);

/*** Transmission/reception ***/

/* Get register flags 1 or 2 */
   byte getFlags(byte nFreg);
/* Get single flag n of flag register 1 or 2 */
   byte getFlag(byte nFreg, byte nFlag);
   void resetFlag(byte nFreg, byte nFlag);   
   
/* Load FIFO with data to transmit (mode FSKOOK)*/
   void dataToSend(byte data[],int len);

/* Set/read FIFO threshold if you use FifoLevel flag */   
   void setFIFOthr(byte nb);
   byte readFIFOthr();

/* Enable/disable (1/0) preamble detection if you use PreambleDetect flag */  
   void preambleDetect(byte yesno);

/* Return payload length if arrived or 0 if not */  
   byte dataAvailable();
/* Read payload bytes into buffer data with max len length */  
   byte dataReceived(byte data[],int len);
/* Discard payload without saving it */   
   void fifoDiscard(); 
 
/* Set sampling rate for RSSI average value estimate (def:2->8 samples).
   val is power of two +1 (Ex. 0->2, 1->4, 2->8 ... 7->256)*/  
   void setRSSIsample(byte val); 
   float readRSSIval();
/* Set/read RSSI threshold if you use RSSI flag */   
   void setRSSIthr(int dBm);
   float readRSSIthr();
   
/* OOK threshold mode: ty=0 fixed, ty=1 peak (def), ty=2 average */
   void setOOKthrMode(byte ty); 
   
   void setOOKbaseThr(int thr); 
   float readOOKbaseThr();

/* OOK threshold trimming: val=0-:-7 dB=0.5-:-6 */
   void setOOKThrStep(byte val);
   void setOOKThrDec(byte val);

/* Bit syncronizer set/disable. In packet mode always enabled by default. */   
   void setBitSync(byte yesno);
   
/********************* LoRa mode ***************************/

/* load FIFO whith data to send (LORA mode) */   
   void setLoraDataToSend(byte data[],byte datalen);
/* read received bytes into buff */   
   int readLoraData(byte buff[], byte blen);
/* discard received bytes */   
   void discardLoraRx();
   
/* Set Spreading Factor code. Spr.Factor values: 6,7,8,9,10,11,12 (def.: 7)*/
   void setLoraSprFactor(byte spf);
   byte getLoraSprFactor();

/* Set Banwidth (from 7.8 kHz to 500 kHz). Code: from 0 to 9 (def.: 7=125 kHz)*/
   void setLoraBw(byte bw);   // bw:code
   byte getLoraBw();          // return code
/* Get Lora Bw in terms of frequency interval */
   float getLoraBwFreq();      // return freq.

/* Set Coding Rate. Values 1=4/5, 2=4/6, 3=4/7, 4=4/8 (def.: 1) */
   void setLoraCr(byte cr);
   byte getLoraCr();

/* Set Preamble length.Value: from 4 to 1024 (def. 12) */   
   void setLoraPreambleLen(unsigned int len);
   unsigned int getLoraPreambleLen();
      
/* Set on/off automatic payload CRC computation/detection  (def.: off)*/
   void setLoraCrc(byte yesno);   
   
/* Symbol rate computation */
   float getSRate();
/* Bit per second (bit rate)*/
   float getLorabps();
   
/* Set on in case of simbol rate < 62/sec (or bps < 1200) */
   void setLoraLowDataRateOptimize(boolean on);
   
/* Set timeout in terms of symbols (bytes) (def 100). 
   So timeout in terms of milliseconds depends on Symbols Rate */
   void setLoraRxByteTout(int nbyte);   
/* Set timeout for data receiving (LORA mode) (def.: 100 symbols) 
   rough conversion is made. So, timeout can be espressed in seconds.milliseconds
   (def.: 0.1Sec)*/   
   void setLoraRxTimeout(float sec);
/* In bytes (symbols) */
   int getLoraRxByteTout();
/* In milliseconds */
   float getLoraRxTimeout();
   
/* set max bytes to receive (def.: 255) */   
   void setLoraMaxRxBuff(byte len);
/* Get LORA flags: 
   RxTimeout 7, RxDone 6, PayloadCrcError 5, ValidHeader 4, 
   TxDone 3, CadDone 2, FhssChangeChannel 1, CadDetected 0   */  
   bool getLoraFlag(byte flag);
/* reset LORA flag */ 
   void clearLoraFlag(byte flag);
/* reset all LORA flags */   
   void clearAllLoraFlag();
/* get LORA flags for timeout or rx done    
   It returns 0 if nothing, 1 if packet received, -1 if timeout */
   int getLoraRxEndFlag();

/* RSSI and SNR estmate  (dBm)*/
   int lastLoraPacketRssi();
   int lastLoraPacketSnr();
   int getLoraRssi();
   int lastLoraPacketSignalPower();
   
/* Set random seed using wideband RSSI noisy measuring */
   unsigned int setRndSeed();
   
/***************************** utilities *********************************/

/* Basic SX1278 register write function */ 
   int SPIwrite(unsigned char address,unsigned char val);
/* Basic SX1278 register read function */ 
   int SPIread(unsigned char address);
   
/* get and set single bit of register */     
   void setRegBit(byte reg,byte n,byte onoff);
   byte getRegBit(byte reg,byte n);
   
/* read and set goup of register bits */   
   void setRegBits(byte reg,byte val,byte bst, byte len);
   byte getRegBits(byte reg,byte bst, byte len);

/* Return a string rapresentation of register content. 
   Format: address(hex) -> bin-rappresentation hex-rapresentation */     
   char* readRegBin(byte reg); 
  
/*************************** AES256 encryption *******************************/
  AES Cr;
  
/* Define a 32 bytes key using an integer value (0-65535). 
   The 32 bytes key will be used by encrypt and decrypt functions */  
  void createKey(unsigned int keyval);
  
/* Encrypt buff (replace each byte) and return the same buffer. 
   Buffer length must be multiple of 16 bytes. The parameter nbk is the number
   of 16 bytes blocks. (I.E. nbk=bufferlen/16)
   This function uses a predefined 32 bytes key */ 
  byte* encryptBuff(byte *buff, int nbk);

/* Decrypt buff (replace each byte) and return the same buffer.
   Buffer length must be multiple of 16 bytes. The parameter nbk is the number
   of 16 bytes blocks. (I.E. nbk=bufferlen/16)
   This function uses a predefined 32 bytes key */   
  byte* decryptBuff(byte *buff,int nbk);         

  byte* getKey();
  private:
  
  bool initSPI();   
  void setBoost(byte yesno);   
  char RegBin[18];
  byte setBit(byte b,byte val,byte bst, byte len);
  byte getBit(byte b,byte bst, byte len);
  
};

extern SX1278 SX;

#endif
