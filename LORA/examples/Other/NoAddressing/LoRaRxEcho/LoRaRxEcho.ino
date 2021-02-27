/* Test radio range LoRa .*/
/* Use together with LoRaTxEcho transmitter sketch to test echo */
/* Receiver: this sketch listen for message coming and echoes it back.
*  It uses continuous receiving. When message arrive stop receiving mode and sends echo.
*  Then restore continuous receiving mode.
* 
*  On serial console are displaied also misured RSSI and SNR values.
*/

#include "LORA.h"
#include <SPI.h>

#define reclen 64       // buffer length
#define format "|Rssi: %d RssiPk: %d SnrPk %d|  "

LORA LR;                // Class LORA instance

boolean SHIELD=true;

char recbuff[reclen];   //receiving buffer
char sendbuff[reclen];  //transmitting buffer

char data[64];          //buffer for RSSI and SNR values

int SF=9;               //Spreading factor value (if changed for test)
int BW=6;               //Bandwidth value (if changed for test)

int CR=4;

float FREQ=434.0;        //default 434.0
int PWR=4;              //transmit power

void setup() 
{
  Serial.begin(9600);
  if (!LR.begin()) 
    {Serial.println("No LoRa shield detected! It can't perform echo!");SHIELD=false;return;} 
  Serial.println("LoRa echo receiver.");   
  SX.setFreq(FREQ);
  SX.setPower(PWR);
  LR.setConfig(SF,BW,CR); //if you want test different configuration (def: 9,6,4)
  showConfig();
  Serial.println("Waiting for message...");
  LR.receiveMessMode();   // set shield in continuous receiving mode
}

void loop() 
{
  if (!SHIELD) {delay(200);return;}
  if (getMess()) {sendEcho();LR.receiveMessMode();} // if mess. send echo and return back to receiving mode
}

boolean getMess()
{
  if (LR.dataRead(recbuff,reclen)<=0) {delay(10);return false;}
  Serial.print("< ");Serial.println(recbuff);
  snprintf(data,63,format,SX.getLoraRssi(),SX.lastLoraPacketRssi(),SX.lastLoraPacketSnr());
  Serial.println(data); 
  return true;
}

void sendEcho()
{
  SX.setState(STDBY);              // stop receiving mode
  delay(300);
  strlcpy(sendbuff,recbuff,reclen);
  Serial.print("> ");Serial.println(sendbuff);
  if (LR.sendMess(sendbuff)<0) Serial.println("Sending error!"); 
}

void showConfig()
{
  Serial.print("Frequence: ");Serial.println(SX.readFreq()); 
  Serial.print("Transmit power (mW): ");Serial.println(SX.getPower(2)); 
  Serial.print("Preamble bytes: ");Serial.print(SX.getLoraPreambleLen());Serial.println("+4");  
  snprintf(data,63,"SpFactor: %d BandW: %d Cr: %d",SX.getLoraSprFactor(),SX.getLoraBw(),SX.getLoraCr());
  Serial.println(data); 
  Serial.print("Rate (byte/sec): ");Serial.println(SX.getSRate());
}


