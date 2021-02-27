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
/* Class LORA. 
*  It semplifies class SX1278 in case of LoRa protocol using.
*  Just use begin function and sendMess or receiveMessMode to communicate.
*  Principal functions:
*  - begin() : initialize shield for LoRa protocol (default power=10dBm).
*    If shield is not here it returns false.
*  - sendMess() : send message (as null terminated string or byte array) 
*  - receiveMessMode() and dataRead() to receive message.
*    receiveMessMode() set shield in continuous receving mode and then you have 
*    to poll message coming with dataRead() in a loop. 
*    
* Author: Daniele Denaro 2015
*/

#ifndef LORA_h
#define LORA_h

#include <SX1278.h>

#define LoraTxTimeout 2000

class LORA
{
  public:

/******* With AES256 cryptography and sender/destination addresses ***********/
  
/**** Initialize ***/

/* Start shield in LoRa mode and prepare 32 byte key for AES256 crypto*/
  bool begin(unsigned int keyval);
/* Change shield in LoRa mode, if it was already started in different mode
   with AES256 key creation */
  void setModeLora(unsigned int keyval);

/****** Define network structure **********/
  
/* Define net structure. This function must be called before defNetAddress
*  Code has to be from 2 to 14 and is the exponent of power of 2.
*  Device address address range will be from 1 and 2^code -1. 0 address is 
*  reserved for broadcasting.
*  Ex.:
*  code 2 -> device addresses allowed 1-3 net max address 16683
*  code 3 -> device addresses allowed 1-7 net max address 8191
*  code 4 -> device addresses allowed 1-15 net max address 4095
*  code 5 -> device addresses allowed 1-31 net max address 2047
*  code 6 -> device addresses allowed 1-63 net max address 1023
*  code 7 -> device addresses allowed 1-127 net max address 511
*  code 8 -> device addresses allowed 1-254 net max address 254
*  :
*  code 13 -> device addresses allowed 1-8191 net max address 7
*  Function returns the resulting maximum net address allowed.
*  Or -1 if code error.
*/
   int defDevRange(unsigned char code);
   
/* Return range of possible net id based on device range defined 
*/    
   int getMaxNetAddress();
   
/* Define net address impliedly used by routines for sending and receive.
*  This function must be called after device range definiction (defDevRange(...))
*  Address "add" must be < than maximum net address returned by defDevRange(...)
*  If is not < then it returns false 
*/
   bool defNetAddress(unsigned int add);  
  

/**** Sending ****/

/* Send buffer mess adding  destination local address and sender local address 
*  Real addresses are composed with network address previously registered. 
*  A random byte is added just to make message unique.
*  Sending address, random byte and message are encoded with AES256 cryptography
*  using predefined 32 key. Length of encoded segment is a multiple of 16 bytes
*  blocks.
*  Destination address is not encoded.
*  EX.: sendNetMess(11,3,buff,bulfflen);
*  
*  Message "mess" can be a null terminated string or a byte array.
*/
  int sendNetMess(unsigned int toSubAdd, unsigned int fromSubAdd, char* mess);
  int sendNetMess(unsigned int destLocalAdd, unsigned int sendLocalAdd, byte *mess, int lmess);

/* Check if any message is on air (channel busy). If true, transmission can be done*/
  bool freeAir();


/**** Receiving ****/

/*
*  Tray to receive one message on the receiver buffer for "tout" milliseconds.
*  Return nessage length. If 0 no message.
*  getMessage() returns the pointer to message.
*  getNetSender() returns the fromSubAddress number
*  getMarker() returns the marker byte 
*  Exit in STDBY state. 
*/
  int receiveNextMessage(unsigned int toSubAdd, unsigned int fromSubAdd, byte *buff, byte maxlen,int tout);

/* Set shield in continuous receiving mode. Then use receive function in a loop  
   to verify if data arrived (use for a continuous receiver)*/
  void receiveMessMode();

/* Receive incoming message into the buffer "buff".
*  If no message is incoming, return 0.
*  If message is incoming:
*  if network address part of message dest. address is different from previously 
*  registered network address, message is discarded and it returns 0.
*  If network address is correct then compare local addressee (destLocalAdd) with local 
*  destination included on message destAdd, and return 0 if message don't concerns
*  this device, but only if local destination address is not 0 (in this case 
*  message is accepted because is a brodcast message).
*  If function sender parameter (sendLocalAdd) is not 0, a check is done to verify 
*  if message is incoming from expected sender; if not, it returns -1
*  (or -2 if sender network address part is incorrect)
*  Finally, return the length of the sole message (without addresses).
*  The message pointer can be obtained by getMess() function.
*  The sender address can be obtained by getSender() function. 
*/
  int receiveNetMess(unsigned int destLocalAdd, unsigned int sendLocalAdd, byte *buff, byte maxlen );

/* Get sender sub-address (local address part of address) in case of network 
   address system */
  unsigned int getSender();

/* Get the clean message buffer (without any other prefix) of last message 
   received  */   
  char* getMessage();
  
/* Get the random marker included on message (both in sent message or in received message)*/  
  byte getMarker();
  

/******** Utilities **************/

/* For convenience. Equivalent to SX.setPower 
*  cod:  1 (7dBm=5mW), 2 (10dBm=10mW), 3 (13dBm=20mW), 4 (17dBm=50mW), 
*        5 (20dBm=100mW)   
*/ 
  void setPower(int cod);
  
/* Setting transmit low power from 2 db to 6 db */  
  void setLowPower(int db); 
   
/* For convenience. Equivalent to SX.setFreq 
*  The parameter freq is in MHz Ex.: 433.93 (default 434.0)
*/ 
  void setFrequency(float freq);

/* For convenience. Like SX.setState(state).
*  If yes==true radio module goes in lowest power state.
*  If yes==false radio module goes in standard standby state; transmitting and
*  receiving starts from this state.
*/ 
  void setSleepState(boolean yes);  
  
/*Configuration 
  Spreading Factor sprf: 6 to 12 (def.: 11) 
  Bandwidth bw: 0 to 9 (def.: 7 (125kHz))
  Coding Rate cr: 1 to 4 (def.: 4)*/   
  void setConfig(byte sprf, byte bw, byte cr);

/* Set on/off automatic payload CRC computation/detection  (def.: off)*/
  void setPayloadCRC(byte yesno);
  
/***** Obsolete and more general***********************************************/

/* Send buffer mess adding a word as addressee (destAdd) and a word as sending 
*  address. 
*  A random byte is added just to make message unique.
*  Sending address, random byte and message are encoded with AES256 cryptography
*  using predefined 32 key. Length of encoded segment is a multiple of 16 bytes
*  blocks.
*  Destination word (two bytes) is not encoded.
*/
  int sendMess(unsigned int destAdd, unsigned int sendAdd, byte *mess, int lmess); 

/* Receive incoming message into the buffer buff
*  If no message is incoming, return 0.
*  If message is incoming compare addressee (that is plain) with destAdd. 
*  If addressee is not correct return 0. (I.E is not a message for the receiver)
*  Then decode message and compare sender with sendAdd. If sender is not correct
*  return -1. But if sendAdd parameter is 0 all messages are accepted.
*  Finally, return the length of the sole message (without addresses).
*  The message pointer can be obtained by getMess() function.
*  The sender address can be obtained by getSender() function. 
*/  
  int receiveMess(unsigned int destAdd, unsigned int sendAdd, byte *buff, byte maxlen); 

/* Get sender address of last message received (a word complete addresss)*/
  unsigned int getLongSender(); 
    
/* DEPRECATED: use defDevRange and defNetAddress */   
/* Set a network address if you like to create a network with net address and
*  a range of device addresses. In this case device adresses range must be a
*  2 power value (8,16,32,64,128,256 ecc.). Besides net address value must be 
*  lower than 0xFFFF/device_range (I.E. 0x1FFF,0xFFF,0x7FF,0x3FF,0x1FF,0xFF ecc) 
*  This function MUST be called before calling sender and receiver functions that
*  use sub-net adressess!
*/
   bool setNetAddress(unsigned int netAdd, unsigned int devRange);  

/* DEPRECATED */   
  unsigned int getNetSender();     

/***************** Basic function (no crypto) **************************/

/* Start shield in LoRa mode */  
  bool begin();
/* Change shield in LoRa mode, if it was already started in different mode */
  void setModeLora();
  
/*** Send ***/

/* Send message (packet) mlen long (or null terminated string).
   Return 0 if ok (sent) or -1 if problem (not sent) */  
  int sendMess(char mess[]);
  int sendMess(byte mess[],byte mlen);

/*** Receive ***/
  
/***************************************************************************/     

/* Data arrived ? If yes, data are copied into mess buffer and function 
   returns number of bytes and mess is a null terminated string.
   If not, function returns 0 (mess = 0 length string)*/
  int dataRead(char mess[],byte maxlen);
/* Buff is a byte array and is not null terminated */
  int dataRead(byte buff[],byte blen);

/*** Further functions ***/

/* CAD monitor for sec seconds and receive message if it is coming. 
   It returns 0 or message length (single packet receiving mode)*/
   int waitForMess(char mess[],byte mlen, float sec);
   int waitForMess(byte buff[],byte blen, float sec);
  
/* Monitor channel for sec seconds. Return true if preamble is detected */
  bool CADmonitor(float sec);  

/* Set timeout (in milliseconds) for each listen period. (Def.: 100)*/  
  void setRxTimeout(int tmillis);
  int getRxTimeout();

  int getReceivedMessLen(){return receivedMessLen;}

  void decodeMess(byte *buff,int len);  
  
  private:
  unsigned int senderAddress;
  byte *receivedMessage;
  int receivedMessLen;
  unsigned int subNetSenderAddress;
  byte marker;
  
  unsigned int netAddress;
  byte r2p;
  unsigned int mask;
  unsigned int netmask;
  unsigned int maxnetadd;
}; 


#endif
