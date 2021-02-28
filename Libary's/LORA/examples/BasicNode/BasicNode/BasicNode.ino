/* This sketch realizes a simple node (for LoRa transmission) that send an event to node 5 when button
 * on pin 6 is pressed, and replays to a command "getAna", from any other node request, sending value of 
 * analogical pin 2.  
 * At starting time it ask by Serial port its address (Id).
 * Other characteristics are default value.
 * Radio characteristics and network addresses are printed on Serial port.
 */

#include "LoraNode.h"       //Include library 

LoraNode Node;              //Instance

bool SHIELD=true;           //Flag to verify correct link with radio module
#define pana 2              //Analogic pin
#define button 6            //Button pin
#define eventHandler 5      //Address of node that receive the button event

void setup() {
  Serial.begin(9600);
  readNodeId();             //Set this node Id 
  if (!Node.begin()) {Serial.println("No LoRa module!");SHIELD=false;return;}
  Node.printConfig();       //Print radio config (for information)
  Node.printAddresses();    //Print network data (for information)
  pinMode(button,INPUT_PULLUP);
}

void readNodeId()           //Read Id for this node by Serial port and set it
{
  Serial.println("Define node address: ? (def.1)");
  while(Serial.available()==0){}
  int id=Serial.parseInt();
  if (id>0) Node.setNodeAdd(id);
}

void loop() {
  if (!SHIELD) return;
  if (Node.newMessAvailable(1000)){prMess();execRequest();} //reception
  if (digitalRead(button)==LOW) {sendEvent();}              //sending
}

void execRequest()
{
  char* req=Node.getMessage();                             //Read message text
  if (strcmp(req,"getAna")==0)                             //if text is "getAna" send value
  {
    int v=analogRead(pana); char sv[10]; itoa(v,sv,10);    //read value and transorm it in char
    int sd=Node.getSender();                               //to whom send ?
    Node.writeMessage(sd,sv,500);                          //send
  }  
}

void sendEvent()
{
  Node.writeMessage(eventHandler,"ButtonOn",500);         //send event "ButtonOn" to appropriate node 
}

void prMess()                                             //print message just for information
{
  Serial.print("From: ");Serial.print(Node.getSender());
  Serial.print(" ");Serial.println(Node.getMessage());
}

