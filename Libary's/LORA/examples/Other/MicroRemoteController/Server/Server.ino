/* Server protected by cryptograph*/
/* Basically it receives activation command by remote control.
*  Can be upgrated with further functionalities.
*  Pins used:
*  - pin for buzzer : D9
*  - pin for led    : D7
*  Basic functionalities in loop:
*  1) Wait for message incoming
*  2) Decode message and sender
*  3) If message is incoming from remote control execute handshake protocol
*  4) Handshake protocol:
*      a) Send back a single byte message with marker received
*      b) If recive OK execute command and replay "OK"
*      c) Else don't execute command and replay "NOK"
*
*  The network id is LORANET and must be the same of transmitter.
*  This device has address 2 (THISADD) (change for other devices from 3 to 15)
*  Key base value is KEYVAL and must be the same of transmitter.
* 
*  On serial console are displaied also misured RSSI and SNR values.
*/

#include "LORA.h"
//#include <SPI.h>

/* Simulated sensors and activators.*/
#define psound 6         // pin for buzzer
#define pinf  7          // pin for signaling led

#define NETADD 234
#define RANGECODE 6
#define THISADD 1
#define KEYVAL 4321

#define MASTER 1
#define REMOTECTRL 10

#define RXTIMEOUT 500    // tens of milliseconds (I.E. RXTIMEOUT*10=milliseconds)

#define reclen 64       // buffer length

#define REMOTECMD "CommandON"

LORA LR;                // Class LORA instance

boolean SHIELD=true;

char recbuff[reclen];   //receiving buffer
char sendbuff[reclen];  //transmitting buffer

char data[80];          //buffer for RSSI and SNR values

int SF=12;               //Spreading factor value (if changed for test)
int BW=7;               //Bandwidth value (if changed for test)
int CR=4;               //Cr mode
int PWR=5;              //transmit power

float FREQ=433.6;        //default 434.0

int Sender;

#define dimlist 10
byte markerlist[dimlist];

void setup() 
{
  digitalWrite(pinf,0);
  pinMode(pinf,OUTPUT);
  pinMode(psound,OUTPUT);
  
  Serial.begin(9600);
  if (!LR.begin(KEYVAL)) 
    {Serial.println("No LoRa shield detected! It can't perform echo!");SHIELD=false;return;} 
  Serial.println("LoRa receiver."); 
  
  LR.defDevRange(6);
  LR.defNetAddress(NETADD);
  LR.setFrequency(FREQ);  
  LR.setPower(PWR);
  LR.setConfig(SF,BW,CR); //if you want test different configuration (def: 9,6,4)

  showAdresses();
  showConfig();
  Serial.println("Waiting for message...");
  LR.receiveMessMode();   // set shield in continuous receiving mode
}

void loop() 
{
  int nb;
  if (!SHIELD) {delay(200);return;}
     // if mess. decode it and return back to receiving mode
  if ((nb=getMess())>0) {decode(nb);LR.receiveMessMode();} 
}

void execCtrlCommand()
{
  sound(500,100,3);
}

int getMess()
{
  int nb=LR.receiveNetMess(THISADD,REMOTECTRL,(byte*)recbuff,reclen);
  if (nb<0){Serial.print("< bytes:");Serial.println(nb);} 
  if (nb<=0) {delay(10);return 0;}
  showSignal(); 
  sound(500,100,1);
  return nb;
}

void decode(int nb)
{
  char *mess=(char*)LR.getMessage();
  Sender=LR.getSender();
  Serial.print("< ");Serial.print("From: ");Serial.print(Sender);
  Serial.print(" Mess: ");Serial.print(LR.getMarker());Serial.print(" ");Serial.println(mess);
  if (Sender==REMOTECTRL) remotectrl();
}

void remotectrl()
{
  char *mess=(char*)LR.getMessage();
  if (strncmp(mess,REMOTECMD,strlen(REMOTECMD))!=0) 
    {Serial.println("Unrecognised command!");reply(REMOTECTRL,"NOK");return;}
  if (!verifyMarker()) 
    {Serial.println("Untrusted command!");reply(REMOTECTRL,"NOK");return;};
  reply(REMOTECTRL,"OK");  
  execCtrlCommand();
}

boolean verifyMarker()
{
  byte marker=LR.getMarker();
  int i;
  for (i=0;i<dimlist;i++) if (marker==markerlist[i]) return false;
  int p=random(0,dimlist);
  markerlist[p]=marker;
  return true;
}

void reply(unsigned int dest,char *mess)
{
  SX.setState(STDBY);              // stop receiving mode
  Serial.print("Replay to ");Serial.println(dest); 
  int messlen=strlen(mess);
  delay(300);
  strlcpy(sendbuff,mess,messlen+1);
  Serial.print("> ");Serial.println(mess);
  if (LR.sendNetMess(dest,THISADD,(byte*)sendbuff,messlen)<0) Serial.println("Sending error!"); 
  else Serial.println("Sent...");
}

void showAdresses()
{
  Serial.print("Net address range: 0 to ");Serial.print(0xFFFF>>RANGECODE);
  Serial.print("  Devices range: 1 to ");Serial.println((int)pow(2,RANGECODE)-1);
  Serial.print("Net address   : ");Serial.println(NETADD);
  Serial.print("My address    : ");Serial.println(THISADD); 
  Serial.print("Remote control: ");Serial.println(REMOTECTRL); 
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



void blinkpinf(int time,int n)
{
  int i;
  byte p=1;
  for (i=0;i<n;i++) {digitalWrite(pinf,p);delay(time); p=p^1;}
  digitalWrite(pinf,0);
}

void sound(int timeon,int timeoff,int n)
{
  int i;
  for (i=0;i<n;i++){digitalWrite(psound,1);delay(timeon);digitalWrite(psound,0);delay(timeoff);}
  digitalWrite(psound,0);
}

