
/********************************************************************************************/
/*
* This example of REMOTEC library scketch is usefull for remote control system detect.
* It displays base impulse and it try to decode data.
*
* Author D.Denaro 2015
*/

#include <SPI.h>
#include "REMOTEC.h"

REMOTEC RC;

int mrssi=-999;
int rssi;

void setup()
{
  Serial.begin(9600);
  RC.begin();
  if (RC.setScannerMode()) Serial.println("Receiving...");
  else Serial.println("Problem! No ready to receive");
}

void loop() 
{
//  rssi=SX.readRSSIval();

  RC.scanImpulses();
//  if (rssi>mrssi)
//  {mrssi=rssi;Serial.print("MaxRSSI: ");Serial.print(mrssi);
//   Serial.print(" ");SX.setOOKbaseThr(mrssi-20);Serial.println(SX.readOOKbaseThr());}
}

