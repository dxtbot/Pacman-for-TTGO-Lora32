/* Remote control sketch.  */
/* ATTENTION: Configured only for LoRa remote control
/* !!! COMPILE as "LiLyPad Arduino USB" platform. !!!
/* To use with remote control hardware.
*  This harware is based on Atmel ATMEGA32u4 (used by Leonardo and LilyPad Arduino USB) and Lora SX1276. 
*  Because 8Mhz clock use only LilyPad Arduino USB board setting for programming.
*  Pins used:
*  - pin for power switch: D6 
*  - pin user led        : D13
*  - pin buzer           : D5
*  - user pin output     : D8
*  - user pin analog inp.: A2
*  
*  (in LORA library SS is defined as D17)
* 
*  Functionalities:
*  1) Push button (parallel with D6) power on. 
*  2) Immediately D6 becomes hight, to keep on the power even if button is released.
*  3) Radio module SX1276 is set and command is sent with random byte.
*  4) Wait for replay with random byte echoed and werify corrispondence.
*  5) Send ok (command will be executed)
*  6) Wait for replay to confirm execution of command
*/

#include <LORA.h>
//#include <SPI.h>

#define pinon 6          // pin for keeping power on
#define psound 5         // pin for buzzer
#define pinf  13         // pin for signaling led

#define COMMAND "CommandON"    // message to send 

//#define RXTIMEOUT 500    // tens of milliseconds (I.E. RXTIMEOUT*10=milliseconds)

#define inplen 64        // sending buffer len 
#define reclen 64        // receiving buffer len 

#define KEY     4321        // key for cripto
#define NETADD  234         // net address < 2048 devices : 63
#define RANGECODE  6        // devices address 1-63 (0 not allowed)
#define THISADD   10        // this device address
#define SERVERADD  1        // destination (server)

LORA LR;                 // Class LORA instance

char inpbuff[inplen];    // sending buffer
char recbuff[reclen];    // receiving buffer

char data[80];           // buffer for configuration and signal values

int SF=12;  //Spreading factor value (if changed for test)
int BW=7;   //Bandwidth value (if changed for test)
int CR=4;   //Cr mode
int PWR=5;  //transmit power

float FREQ=433.6;        //default 434.0

void setup() 
{
  pinMode(pinon,OUTPUT);
  digitalWrite(pinon,1);
  pinMode(pinf,OUTPUT);
  pinMode(psound,OUTPUT);    
  digitalWrite(pinf,1);
  digitalWrite(psound,1);
//  delay(8000);
  Serial.begin(1200);
  if (!LR.begin(KEY)) {sketchend(1);return;}
// while(!Serial){ digitalWrite(psound,1);};
  Serial.println("LoRa command transmitter.");

  LR.defDevRange(RANGECODE);
  LR.defNetAddress(NETADD);
  LR.setFrequency(FREQ);    
  LR.setPower(PWR);
  LR.setConfig(SF,BW,CR);       // if you want test different configuration (def: 9,6,4)

  showAdresses();
  showConfig();
  sound(200,10,1);

  int seed=analogRead(0)*analogRead(1); //random seed using analog input noise
  randomSeed(seed);                     //random seed for random marker
  
  sendCommand();
  if (!getCommandReply()) {sketchend(2);return;}
  sketchend(0);
}

void loop() 
{
}

void sketchend(int err)
{
    switch (err)
    {
      case 0: Serial.println("Command executed!OK!");blinkpinf(500,3);sound(400,50,2);break;
      case 1: Serial.println("No LoRa shield detected! Stop!");blinkpinf(100,3);break;
      case 2: Serial.println("Command not executed! Stop!"); blinkpinf(100,10);sound(200,50,3);break;
    }
  digitalWrite(pinf,0);
  digitalWrite(psound,1);
  digitalWrite(pinon,0);
}

void sendCommand()
{
  sendBuff(COMMAND);
}

boolean getCommandReply()
{
  if (!getReply()) return false;
  char* mess=(char*)LR.getMessage();
  Serial.print("< ");Serial.println(mess);
  if (strncmp(mess,"NOK",3)==0) return false; 
  return true; 
}

void sendBuff(char mess[])
{
  SX.setState(STDBY);
  strlcpy(inpbuff,mess,inplen);
  int f=LR.sendNetMess(SERVERADD,THISADD,(byte*)inpbuff,strlen(mess)); 
  Serial.print("> ");Serial.print(LR.getMarker());Serial.print(" ");Serial.println(inpbuff);
  if (f<0) Serial.println("Error in transmission!");
  SX.setState(STDBY);
}

boolean getReply()
{
  LR.receiveMessMode();
  boolean OK=false;
  int i;
  for (i=0;i<RXTIMEOUT;i++)
    {if (LR.receiveNetMess(THISADD,SERVERADD,(byte*)recbuff,reclen)>0) {OK=true;break;} delay(10);}
  if (!OK) return false;
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

void sound(int timeon,int timeoff,int n)
{
  int i;
  for (i=0;i<n;i++){digitalWrite(psound,0);delay(timeon);digitalWrite(psound,1);delay(timeoff);}
  digitalWrite(psound,1);
}

void showAdresses()
{
  Serial.print("Net address range: 0 to ");Serial.print(0xFFFF>>RANGECODE);
  Serial.print("  Devices range: 1 to ");Serial.println((int)pow(2,RANGECODE)-1);
  Serial.print("Net address   : ");Serial.println(NETADD);
  Serial.print("My address    : ");Serial.println(THISADD); 
  Serial.print("Dest. Server  : ");Serial.println(SERVERADD); 
}

void showConfig()
{
  Serial.print("Replay timeout (millisec.): ");Serial.println(RXTIMEOUT*10); 
  Serial.print("Frequence: ");Serial.println(SX.readFreq()); 
  Serial.print("Transmit power (mW): ");Serial.println(SX.getPower(3)); 
  Serial.print("Preamble bytes: ");Serial.print(SX.getLoraPreambleLen());Serial.println("+4"); 
  Serial.print("SpFactor: ");Serial.println(SX.getLoraSprFactor());
  Serial.print("BandWcode: ");Serial.print(SX.getLoraBw());Serial.print(" -> kHz ");Serial.println(SX.getLoraBwFreq()); 
  Serial.print("Cr_code: ");Serial.println(SX.getLoraCr());
  Serial.print("Rate (byte/sec): ");Serial.println(SX.getLorabps());
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


