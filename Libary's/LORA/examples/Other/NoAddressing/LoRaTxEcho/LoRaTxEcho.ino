/* Test radio range LoRa . */
/* Use together with LoRaRxEcho receiver sketch to test echo */
/* Transmitter: this sketch sends the MESS string and wait for echo sent by receiver.
*  It waits for reply for RXTIMEOUT*10 milliseconds.
*  Message is sent using a push-button (to ground) on pin "pingo".
*  Result is showed by serial console and by led (on pin "pinf") and buzzer (on pin "psound"):
*  - message sent   : short blink and 1 short sound 
*  - no replay      : shortest blink and 3 short sound.
*  - reply incorrect: long blinking and 2 long sound .
*  - reply correct  : led on and 1 long sound
* 
*  On serial console are displaied also misured RSSI and SNR values.
*/

#include <LORA.h>
#include <SPI.h>

#define psound 6         // pin for buzzer
#define pingo 8          // pin for push-button
#define pinf  7          // pin for signaling led

#define MESS "Simple echo test"    // message to send and receive

#define RXTIMEOUT 500    // tens of milliseconds (I.E. RXTIMEOUT*10=milliseconds)

#define inplen 64        // sending buffer len 
#define reclen 64        // receiving buffer len 

LORA LR;                 // Class LORA instance

char inpbuff[inplen];    // sending buffer
char recbuff[reclen];    // receiving buffer

char data[64];           // buffer for RSSI and SNR
#define format "|Rssi: %d RssiPk: %d SnrPk %d|  "

int SF=9;  //Spreading factor value (if changed for test)
int BW=6;  //Bandwidth value (if changed for test)

int CR=4;

float FREQ=434.0;        //default 434.0
int PWR=4; //transmit power

boolean SHIELD=true;

void setup() 
{
  pinMode(pingo,INPUT_PULLUP);
  digitalWrite(pinf,0);
  pinMode(pinf,OUTPUT);
  pinMode(psound,OUTPUT);
  Serial.begin(9600);
  if (!LR.begin()) 
    {Serial.println("No LoRa shield detected! Stop!");SHIELD=false;return;}
  Serial.println("LoRa echo transmitter.");
  SX.setFreq(FREQ);
  SX.setPower(PWR);
  LR.setConfig(SF,BW,CR);       // if you want test different configuration (def: 9,6,4)
  showConfig();
  strlcpy(inpbuff,MESS,inplen);
  Serial.print("Close pin ");Serial.print(pingo);Serial.println(" to ground to send message");  
}

void loop() 
{
  delay(200);
  if (!SHIELD) return;
  if (getInput()) {sendBuff();getReplay();}
}



boolean getInput()
{
  if (digitalRead(pingo)>0) return false;
  return true;
}

void sendBuff()
{
  sound(300,1);
  blinkpinf(100,10);
  digitalWrite(pinf,LOW);
  
  Serial.print("> ");Serial.println(inpbuff);
  int f=LR.sendMess(inpbuff); 
  if (f<0) Serial.println("Error in transmission!");

  SX.setState(STDBY);
}

void getReplay()
{
  LR.receiveMessMode();
  boolean OK=false;
  int i;
  for (i=0;i<RXTIMEOUT;i++)
    {if (LR.dataRead(recbuff,reclen)>0) {OK=true;break;} delay(10);}
  if (!OK) {Serial.println("No replay!");blinkpinf(50,20);sound(300,3);return;}
  snprintf(data,63,format,SX.getLoraRssi(),SX.lastLoraPacketRssi(),SX.lastLoraPacketSnr());
  Serial.println(data);
  Serial.print("< ");Serial.println(recbuff); 
  int inc=strlen(inpbuff);
  if (strncmp(recbuff,inpbuff,inc)==0) {digitalWrite(pinf,HIGH);sound(1000,1);}
  else {blinkpinf(500,4);sound(500,2);}
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

void showConfig()
{
  Serial.print("Replay timeout (millisec.): ");Serial.println(RXTIMEOUT*10); 
  Serial.print("Frequence: ");Serial.println(SX.readFreq()); 
  Serial.print("Transmit power (mW): ");Serial.println(SX.getPower(3)); 
  Serial.print("Preamble bytes: ");Serial.print(SX.getLoraPreambleLen());Serial.println("+4"); 
  snprintf(data,63,"SpFactor: %d BandW: %d Cr: %d",SX.getLoraSprFactor(),SX.getLoraBw(),SX.getLoraCr()); 
  Serial.println(data); 
  Serial.print("Rate (byte/sec): ");Serial.println(SX.getSRate());
}


