/* Test LoRa . Use together with LoRaRXTestRange receiver sketch to test range */
/* Transmitter: this sketch sends the MESS string on loop.
*  Transmit every TIMER (5sec) for TOTSTEP times
*/

#include <LORA.h>

#define psound 6         // pin for buzzer
#define pingo 8          // pin for push-button
#define pinf  7          // pin for signaling led

#define MESS "Simple range test"    // message to send and receive

#define RXTIMEOUT 1000    // tens of milliseconds (I.E. RXTIMEOUT*10=milliseconds)

#define inplen 64        // sending buffer len 

LORA LR;                 // Class LORA instance

char inpbuff[inplen];    // sending buffer

char data[80];           // buffer for RSSI and SNR

int SF=12;  //Spreading factor value (if changed for test)
int BW=5;   //Bandwidth value (if changed for test)
int CR=4;   //Ridondance

float FREQ=433.5;        //default 434.0
int PWR=2;               //transmit power

boolean SHIELD=true;

#define TIMER 5000     //sending interval
#define TOTSTEP 360     //total messages

int counter=0;

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

  LR.setFrequency(FREQ);
  LR.setPower(PWR);
  LR.setConfig(SF,BW,CR);       // if you want test different configuration (def: 9,6,4)

  showConfig();
  strlcpy(inpbuff,MESS,inplen);
  Serial.print("Close pin ");Serial.print(pingo);Serial.println(" to ground to send message");  
}

void loop() 
{
  delay(200);
  if (!SHIELD) return;
  if (getInput()) {sendBuff();}
  else LR.setSleepState(true);
}



boolean getInput()
{
//  if (digitalRead(pingo)>0) return false;
//  if (Serial.available()==0) return false;
//  Serial.read();
  if (counter>TOTSTEP) return false;
  counter++; 
  delay(TIMER);
  return true;
}

void sendBuff()
{
  sound(300,1);
  blinkpinf(100,10);
  digitalWrite(pinf,LOW);
  
  Serial.print(counter);Serial.print("  > ");Serial.println(inpbuff);
  int f=LR.sendMess(inpbuff); 
  if (f<0) Serial.println("Error in transmission!");
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



