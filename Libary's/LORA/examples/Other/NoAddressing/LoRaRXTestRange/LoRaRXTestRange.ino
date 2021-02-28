/* Test LoRa . Use together with LoRaTXTestRange transmitter sketch to test range */
/* Receiver: this sketch listen for message coming .
*  It uses continuous receiving. 
* 
*  On serial console are displaied also misured RSSI and SNR values.
*/

#include "LORA.h"

#define psound 6         // pin for buzzer
#define pingo 8          // pin for push-button
#define pinf  7          // pin for signaling led

#define reclen 64       // buffer length

LORA LR;                // Class LORA instance

boolean SHIELD=true;

char recbuff[reclen];   //receiving buffer

char data[80];          //buffer for RSSI and SNR values

int SF=12;              //Spreading factor value (if changed for test)
int BW=5;               //Bandwidth value (if changed for test)
int CR=4;               //redundancy

float FREQ=433.5;        //default 434.0
int PWR=2;               //transmit power

void setup() 
{
  pinMode(pingo,INPUT_PULLUP);
  digitalWrite(pinf,0);
  pinMode(pinf,OUTPUT);
  pinMode(psound,OUTPUT);  
  Serial.begin(9600);
  if (!LR.begin()) 
    {Serial.println("No LoRa shield detected! Exit!");SHIELD=false;return;} 
  Serial.println("LoRa echo receiver.");  

//  SX.setRegBit(0x26,2,1); //auto LNA

  LR.setFrequency(FREQ);
  LR.setPower(PWR);
  LR.setConfig(SF,BW,CR); //if you want test different configuration (def: 9,6,4)
  showConfig();
  Serial.println("Waiting for message...");
  LR.receiveMessMode();   // set shield in continuous receiving mode
//  pinMode(13,OUTPUT);
//  digitalWrite(13,1);
}

void loop() 
{
  if (!SHIELD) {delay(200);return;}
  if (getMess()) {digitalWrite(pinf,1);sound(500,1);digitalWrite(pinf,0);}
}

boolean getMess()
{
  if (LR.dataRead(recbuff,reclen)<=0) {delay(10);return false;}
  Serial.print("< ");Serial.println(recbuff);
  showSignal();
  return true;
}

void blinkpinf(int time,int n)
{
  int i;
  byte p=1;
  for (i=0;i<n;i++) {digitalWrite(pinf,p);delay(time); p=p^1;}
  digitalWrite(pinf,0);
}

void sound(int time,int n)
{
//  return; //uncomment if you like sound
  int i;
  for (i=0;i<n;i++){digitalWrite(psound,1);delay(time);digitalWrite(psound,0);delay(100);}
  digitalWrite(psound,0);
}

#define format "|Rssi: %d Snr: %d -> Signal: %d (dBm) 0.%dE%d (mW)|"
void showSignal()
{
  int PkRssi=SX.lastLoraPacketRssi();
  int PkSnr=SX.lastLoraPacketSnr();
  int Signal=PkRssi+PkSnr;
  float fSignal=(float)Signal/10;
  int esp=int(fSignal);float man=pow(10,(fSignal-esp)); int iman=man*100;
  snprintf(data,80,format,PkRssi,PkSnr,Signal,iman,esp); 
  Serial.println(data); 
}

void showConfig()
{
  Serial.print("Frequence: ");Serial.println(SX.readFreq()); 
  Serial.print("Transmit power (mW): ");Serial.println(SX.getPower(2)); 
  Serial.print("Preamble bytes: ");Serial.print(SX.getLoraPreambleLen());Serial.println("+4"); 
  Serial.print("SpFactor: ");Serial.println(SX.getLoraSprFactor());
  Serial.print("BandWcode: ");Serial.print(SX.getLoraBw());Serial.print(" -> kHz ");Serial.println(SX.getLoraBwFreq()); 
  Serial.print("Cr_code: ");Serial.println(SX.getLoraCr());
  Serial.print("Rate (bps): ");Serial.print(SX.getLorabps());
  Serial.print(" Byte/sec: ");Serial.println(SX.getLorabps()/8);      
}



