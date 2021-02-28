/* Test LoRa network. Use together with LoRaRxAddressing receiver sketch to test*/
/* Transmitter: this sketch is a network star center.
*  It listens to console input and to receive input.
*  If message is inputted (in the format: addressee message), schetch sends it 
*  and waits for the replay (and prints it on console).
*  If undemanded packet is come in it is printed on console.
*  Messages concern some simulates sensor and actuators on receivers (buzzer,led,
*  push-button)
*  Message will be one of these commands:
*     echo=aaa... (where aaa... is the string that will be echoed)
*     buzzer=m,n  (where m is the pulse length and n are the repetitions)
*     led=on|off
*     getbutton
*  Replay will be "ok" if message received (or the value of button : 0|1, or the
*  echo of string) 
*  Ex.: 2 buzzer=300,3  (to device 2 : buzzer 3 times 300 milliseconds)
*  Unsolicited messages are sent by devices (receivers) if button is pushed.  
*     
*  The network id is LORANET and must be the same on receivers.
*  This network center has address 1 (MYADD)
*  Key base value is KEYVAL and must be the same on receivers.
*  Devices address (receivers) can be from 2 to 15.
* 
*  On serial console are displaied also misured RSSI and SNR values.
*/

#include <LORA.h>

/*** same values for whole net ***/
#define DEVRANGE 4       // 1 to 15 
#define LORANET 333      // choosed from 0 to 16383
#define KEYVAL 111       // criptography key (mus be the same for all devices.
#define MASTER 1         // server address 

/*** unique value: different for each device ***/
#define MYADD 1          // is master


#define RXTIMEOUT 500    // tens of milliseconds (I.E. RXTIMEOUT*10=milliseconds)

#define inplen 64        // sending buffer len 
#define reclen 64        // receiving buffer len 

LORA LR;                 // Class LORA instance

#define LN 10

char inpbuff[inplen];    // sending buffer
char recbuff[reclen];    // receiving buffer

char data[64];           // buffer for RSSI and SNR
#define format "|Rssi: %d RssiPk: %d SnrPk %d|  "

int SF=9;  //Spreading factor value (if changed for test)
int BW=6;  //Bandwidth value (if changed for test)

int PWR=2; //transmit power

boolean SHIELD=true;

unsigned dest;
char *mess;

void setup() 
{
  Serial.begin(9600);
  
  if (!LR.begin(KEYVAL)) 
    {Serial.println("No LoRa shield detected! Stop!");SHIELD=false;return;}

  LR.defDevRange(DEVRANGE);
  LR.defNetAddress(LORANET);

  Serial.print("Net address  : ");Serial.println(LORANET);
  Serial.print("Devices range: ");Serial.println(int(pow(2,DEVRANGE)-1));
  Serial.print("My address   : ");Serial.println(MYADD);
    
  Serial.println("LoRa transmitter ready...");
  SX.setPower(PWR);
//  LR.setConfig(SF,BW,4);       // if you want test different configuration (def: 9,6,4)
  showConfig();
  LR.receiveMessMode();   // set shield in continuous receiving mode  
}

void loop() 
{
  delay(200);
  if (!SHIELD) return;
  int nb;
  if ((nb=getInput())>0) {if (sendBuff(nb))getReplay();}
  if ((nb=getMess())>0) {decode(nb);}
}

boolean getInput()
{
  if (Serial.available()==0) return 0;
  int nb=Serial.readBytesUntil(LN,inpbuff,inplen);
  inpbuff[nb]=0;
  mess=decodeInp(nb);
  nb=strlen(mess);
  return nb;
}

int getMess()    // in this case function recieves messages from every devices (sender=0)
{
  int nb;
  if ((nb=LR.receiveNetMess(MYADD,0,(byte*)recbuff,reclen))<=0) {delay(10);return 0;}
  snprintf(data,63,format,SX.getLoraRssi(),SX.lastLoraPacketRssi(),SX.lastLoraPacketSnr());
  Serial.println(data); 
  unsigned int sender=LR.getNetSender(); 
  Serial.print("From ");Serial.print(sender);Serial.print(" < ");
  Serial.println((char*)LR.getMessage());   
  return nb;
}

void decode(int nb)
{
}

boolean sendBuff(int nb)
{
  Serial.print("To ");Serial.print(dest);Serial.print(" > ");Serial.println(mess);
  int f=LR.sendNetMess(dest,MYADD,(byte*)mess,nb); 
  if (f<0) Serial.println("Error in transmission!");
  else Serial.println("Sent...");
  SX.setState(STDBY);
  return true;
}

void getReplay()
{
  LR.receiveMessMode();
  boolean OK=false;
  int i;
  for (i=0;i<RXTIMEOUT;i++)
    {if (LR.receiveNetMess(MYADD,0,(byte*)recbuff,reclen)>0) {OK=true;break;} delay(10);}
  if (!OK) {Serial.println("No replay!");return;}
  snprintf(data,63,format,SX.getLoraRssi(),SX.lastLoraPacketRssi(),SX.lastLoraPacketSnr());
  Serial.println(data);
  unsigned int sender=LR.getNetSender();
  Serial.print("From ");Serial.print(sender);Serial.print(" < ");Serial.println((char*)LR.getMessage()); 
}

char* decodeInp(int nb)
{
  char c;
  sscanf(inpbuff,"%d %c",&dest,&c);
  mess=strchr(inpbuff,c);  
  return mess;
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


