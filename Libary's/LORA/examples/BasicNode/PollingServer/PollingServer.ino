/* This sketch realizes an example of polling server in a star network.
 * This server (id = 1) asks every "maxdevice" nodes (starting from node 2) the sensor value.
 * In this example the value is just displayed.
 * If node doesn't replay in 1000 milliseconds a warning is printed. 
 */

#include "LoraNode.h"       //Include library 

LoraNode Node(1);           //Instance node 1 (server)

bool SHIELD=true;           //Flag to verify correct link with radio module

int maxdevice=15;

int timing=10000;           //server scans all devices every 10 seconds

void setup() {
  Serial.begin(9600);
  if (!Node.begin()) {Serial.println("No LoRa module!");SHIELD=false;return;}
  Node.printConfig();       //Print radio config (for information)
  Node.printAddresses();    //Print network data (for information)
  Serial.println("Start polling server");
}

void loop() {
  if (!SHIELD) return;
  bool ack=false;
  for (int i=2;i<=maxdevice;i++)
  {
    Node.writeMessage(i,"GiveMeVal",100); //Send interrogation
    ack=Node.newMessAvailable(i,1000);    //wait message from dev i for 1 sec  
    if (!ack) norep(i);                   //if no reply (timeout)
    else getVal(i);                       //if replay manage it
  }
  delay(timing);
}

void getVal(int dev)
{
  char* mess=Node.getMessage();
  int val=atoi(mess);
  manageMess(dev,val);
}

void manageMess(int dev,int val)
{
  Serial.print("Device: ");Serial.print(dev);Serial.print("  value: ");Serial.println(val);
  //or change with your code for devices replay
}

void norep(int dev)
{
  Serial.print("Device ");Serial.print(dev);Serial.println(" no responding!");
}

