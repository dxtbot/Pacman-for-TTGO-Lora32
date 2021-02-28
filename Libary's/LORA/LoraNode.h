 /*
  Copyright (c) 2018 Daniele Denaro.  All right reserved.

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
/* Class LoraNode.
*  Specialised to make a LoRa node in a simple way.
*  LoRa node are capable of 2-directional transmission but half-duplex.
*  Transmission is crypted (AES256).
*  LoRa node can links n nodes inside of a network identified by an network Id.
*  The number of addressable nodes inside the network defines the range of 
*  possible Id number for the network.
*  For example, if devices addressable are 15(1 to 15), network Id can be choosed
*  from 0 to 4095. Default: max devices=15, network Id=2345.
*  
*  Principal functions:
*  - LoraNode(int thisnode) : instance a LoraNode with address=thisnode
*  - writeMessage(int dest, char* message, int timeout) : send a message to dest
*                node and wait for timeout milliseconds if air busy.
*  - newMessAvailable(int timeout) wait incoming message for timeout milliseconds
*  
*  The received message is returned by getMessage() function and sender is 
*  returned by getSender() function.
*  Sending and receiving message can work with automatic acknowledge. That is, 
*  sending message waits few milliseconds for "AK" message replay and receiving 
*  sends replay "AK" message.
*  Any message contain a random byte that ca be read by getMarker() function.               
*    
* Author: Daniele Denaro 2018
* Version 3.0
*/



#ifndef LoraNode_h
#define LoraNode_h
#include "LORA.h"

#define defNETADD     2345       //Default network Id 
#define defNUMDEVCODE 4          //Default device code (= 15 max devices)
#define defNODEADD    1          //Default node Id
#define defKEYVAL     4321       //Default cryptography key

#define defFREQ  433.6           //Default frequency (MHz)

#define defSF 10                 //Default spreading factor code
#define defBW  8                 //Default band width code
#define defCR  4                 //Default redundant bits

#define defPWR 2                 //Default transmission power code (= 10dBm)

#define receiveBufferLen 64      //Default length of receiving buffer

class LoraNode
{
  public:
  
  LORA LR;
  
/*************** Basic *******************/  
  
/* Lora node instance with nodeAdd as node id (default 1)*/
  LoraNode(); //if EEPROM position 0 and 1 are not 255 load nodeAdd from EEPROM
  LoraNode(unsigned int nodeAdd);

/* Start Lora radio module. If can't use module it returns false. */ 
  bool begin();
  
/* Send message to dest node (of this network id) */
  bool writeMessage(int dest,char* message,int timeout);

/* Wait timeout (1 to 32767) milliseconds for incoming message */  
/* Or wait until message arrives (no timeout) if timeout=0 */  
/* NB default receiving buffer is 64 bytes long. Use changeMessageBufferLen()*/
/* if you need message longher */
  bool newMessAvailable(int timeout);  //from anyone
  bool newMessAvailable(int from,int timeout);  //from "from"
    
/* Get message received */  
  char* getMessage();
/* Get sender node id of received message */   
  int  getSender();
  
/* Get(random) marker byte of message received or sent */  
  int  getMarker();  
  
/******** change default and get info ***********/ 
 
/* Set automatic replay for mess. received and wait for replay of mess. sent (def. yes)*/
  void setAutomaticAck(bool yes); 

/* Set this node address (node id)*/  
  void setNodeAdd(unsigned int nodeAdd);
/* Return address of this node */  
  unsigned int getNodeAdd();
/* Save node id on EEPROM */ 
  void saveNodeAdd(); 
  
/* Set power (change default value) (code: 1 to 5 -> 7 to 20 dBm) */ 
  void setPower(byte code);
/* Get power code set */  
  int getPowerCode();

/* Print (on Serial) radio module configuration */  
  void printConfig();
/* Print (on Serial) adresses (this node and network id)  */
  void printAddresses();

/* Set crypto key (change default value)*/  
  void setKey(int key);
  
/* Set network Id (change default value)*/
  void setNetId(unsigned int id);
/* Get network Id. */ 
  unsigned int getNetId();
/* Get range of network Id, compatible with devices range set */  
  unsigned int getMaxId();
  
/* Set max number of addressable devices (2^code). That is define network config.*/
  void setMaxDevices(byte code);
/* Get code defining number of addressable devices (=2^code)*/
  unsigned int getMaxDevices();

/* Set frequence (MHz)(change default value) */  
  void setFrequency(float nMHz);
/* Get frequence utilized (MHz) */  
  float getFrequency();
  
/* Set spreading factor (change default value) (code: 6 to 12) */
  void setSpreadingFactor(byte code);
/* Set BW (change default value) */  
  void setBandWidth(byte code);
  
/* Test if no communication on air (same freq.,spreading factor,band width)*/
  bool freeAir();

/* Change default buffer length that is 64 bytes */  
  void changeMessageBufferLen(int maxMesslen); 

/* Utility for EEPROM management */  
  void loadNetConfig();   
  void saveNetConfig();   //node_id[0,1],net_id[2,3],numdevcode[4],key[5,6]
  void loadRadioConfig();
  void saveRadioConfig(); //freq[7,8,9,10],sf[11],bw[12],powcode[13]   
  void resetEEPROM();
  
/************** Expansion for binary data ***************/
/* Send a binary message */
  bool writeMessageByte(int dest,byte message[],int messlen,int timeout);
/* Receive a bynary message 
  (it doesn't use default buffer, but a supplied buffer instead) */  
  bool newMessByteAvailable(int from,byte binbuff[],int bufflen,int timeout);
/* Get message pointer (byte) */  
  byte* getMessageByte();
/* Get recent bytes number received */  
  int getNumByteReceived();    
/********************************************************/  
  
  private:
  void initDefault();
  bool incomingMessage(int from,int timeout);
  
  unsigned int NETADD;
  unsigned int NUMDEVCODE;
  unsigned int NODEADD;
  unsigned int KEYVAL;

  float FREQ;

  byte SF;
  byte BW;
  byte CR;
  byte PWR; 
  
  bool factive; 
  
  int bufflen;
  byte* recbuff;

  bool autoAK;

};

#endif
