/* This sketch realizes a simple gateway for binary data.
 * Binary data are read from serial port and sent through LoRa net 
 * In the same time data are read from LoRa net and write to serial port.
 */

#include "LoraNode.h"       //Include library 

bool SHIELD=true;           //Flag to verify correct link with radio module
#define thisnode   2        //Address of this node
#define pairednode 5        //Address of companion node

#define inbufflen  64
#define oubufflen  64

byte inbuff[inbufflen];
byte oubuff[oubufflen];

int nbtosend;

LoraNode Node(thisnode);    //Instance 

void setup() {
  Serial.begin(9600);
  if (!Node.begin()) {Serial.println("No LoRa module!");SHIELD=false;return;}
  Node.printConfig();       //Print radio config (for information)
  Node.printAddresses();    //Print network data (for information)
}

void loop() {
  if (!SHIELD) return;
  if (dataAvailable()) sendData();                                            //sending
  if (Node.newMessByteAvailable(pairednode,inbuff,inbufflen,1000)) recData(); //reception
}

bool dataAvailable()
{
  nbtosend=Serial.available();
  if (nbtosend<=0) return false;
  if (nbtosend>oubufflen) {Serial.println("Err.len!");return false;}
  return true;
}

void sendData()
{
  Serial.readBytes(oubuff,nbtosend);
  Node.writeMessageByte(pairednode,oubuff,nbtosend,500);
}

void recData()
{
  byte* data=Node.getMessageByte();
  int nb=Node.getNumByteReceived();
  Serial.write(data,nb);
}


