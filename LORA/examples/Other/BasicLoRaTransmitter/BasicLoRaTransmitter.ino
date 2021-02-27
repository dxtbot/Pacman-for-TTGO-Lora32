/* Basic example : transmition
*  Read monitor input and send it 
*/

#include <LORA.h>

/*** Network values ***/
#define DEVRANGE 4       // 1 to 15 
#define LORANET 333      // choosed from 0 to 16383
#define KEYVAL 111       // criptography key (mus be the same for all devices.

/*** sender address ***/
#define MYADD 4  

/*** receiver address ***/
#define TOADD 6

#define reclen 64   //console input buffer
char recbuff[reclen];

LORA LR;             //class instance

void setup() 
{
  Serial.begin(9600);
  if (!LR.begin(KEYVAL)) //initialise LoRa radio
    {Serial.println("No LoRa shield detected! Stop!");return;}  
  /* Network definition */
  LR.defDevRange(DEVRANGE);
  LR.defNetAddress(LORANET);  
  Serial.println("LoRa transmitter ready...");
}

void loop() 
{
  int nb=readConsole();
  if (nb>0) LR.sendNetMess(TOADD,MYADD,(byte*)recbuff,nb);
}

#define LN 10     // lenefeed (end record)
int readConsole()
{
  if (Serial.available()==0) return 0;
  int nb=Serial.readBytesUntil(LN,recbuff,reclen);
  return nb;
}

