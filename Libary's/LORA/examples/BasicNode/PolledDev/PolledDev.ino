/* This sketch acts as a remote device. This device read a value on analogical pin "pana"
 * and send it to the polling server (id 1) when interrogated. 
 * This sketch uses a defined address 2 for simplicity. 
 * Other devices need a different node id or load it from EEPROM.
 */

#include "LoraNode.h"       //Include library 

LoraNode Node(2);           //Instance node 2

bool SHIELD=true;           //Flag to verify correct link with radio module
#define pana 2              //Analogic pin

void setup() {
  Serial.begin(9600);
  if (!Node.begin()) {Serial.println("No LoRa module!");SHIELD=false;return;}
  Node.printConfig();       //Print radio config (for information)
  Node.printAddresses();    //Print network data (for information)
  Serial.println("Start remote device");
}

void loop() {
  if (!SHIELD) return;
  if (Node.newMessAvailable(1,20000)) {replay();}
}

void replay()
{
  int v=analogRead(pana);       //read value
  char sv[10]; 
  itoa(v,sv,10);                //transorm value in char
  Node.writeMessage(1,sv,500);  //send
}

