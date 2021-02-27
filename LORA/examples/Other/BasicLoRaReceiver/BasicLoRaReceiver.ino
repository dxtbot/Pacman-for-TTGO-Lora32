/* Basic example : receiving
*  Read incoming message and show it 
*/

#include <LORA.h>

/*** Network values ***/
#define DEVRANGE 4       // 1 to 15 
#define LORANET 333      // choosed from 0 to 16383
#define KEYVAL 111       // criptography key (mus be the same for all devices.

/*** receiver address ***/
#define MYADD 6  

#define reclen 64   //receiver buffer
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
  LR.receiveMessMode();   // radio moldule in receiver mode 
  Serial.println("LoRa receiver ready...");
}

void loop() 
{
  if (readLoRa()>0) 
  {
    int sender=LR.getNetSender();
    char *mess=LR.getMessage();
    Serial.print("From: ");Serial.print(sender);Serial.print(" > ");Serial.println(mess);
  }
}

int readLoRa()   //receive from anyone on net
{
  int nb=LR.receiveNetMess(MYADD,0,(byte*)recbuff,reclen);
  return nb;
}

