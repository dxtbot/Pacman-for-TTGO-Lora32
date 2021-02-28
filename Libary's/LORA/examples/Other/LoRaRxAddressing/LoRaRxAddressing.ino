/* Test LoRa network. Use together with LoRaTxAddressing transmitter sketch to test*/
/* Receiver : this sketch simulates an addressable device.
*  It listens for incoming messages and decodes commands.
*  Or it reads push-button state and if it is pushed sends unsolicited message 
*  with warning. 
*  In case of arrived message, the sketch executes comand and sends replay.
*  Available commands:
*     echo=aaa...  (replay string aaa...)  
*     buzzer=m,n   (sound for m milliseconds, n times) (and replay ok)
*     led=on|off   (switch on/off led and replay ok)
*     getbutton    (read button value and reply it)
*
*  The network id is LORANET and must be the same of transmitter.
*  This device has address 2 (MYADD) (change for other devices from 3 to 15)
*  Key base value is KEYVAL and must be the same of transmitter.
* 
*  On serial console are displaied also misured RSSI and SNR values.
*/

#include "LORA.h"

/* Simulated sensors and activators.*/
#define psound 6         // pin for buzzer
#define pingo 8          // pin for push-button
#define pinf  7          // pin for signaling led

/*** same values for whole net ***/
#define DEVRANGE 4       // 1 to 15 
#define LORANET 333      // choosed from 0 to 16383  
#define KEYVAL 111       // criptography key (mus be the same for all devices.
#define MASTER 1         // server address 

/*** unique value: different for each device ***/
#define MYADD 2          // address of this device (change for other device)

#define reclen 64       // buffer length
#define format "|Rssi: %d RssiPk: %d SnrPk %d|  "

LORA LR;                // Class LORA instance

boolean SHIELD=true;

char recbuff[reclen];   //receiving buffer
char sendbuff[reclen];  //transmitting buffer

char data[64];          //buffer for RSSI and SNR values

int SF=9;               //Spreading factor value (if changed for test)
int BW=6;               //Bandwidth value (if changed for test)

int PWR=2;              //transmit power

void setup() 
{
  pinMode(pingo,INPUT_PULLUP);
  digitalWrite(pinf,0);
  pinMode(pinf,OUTPUT);
  pinMode(psound,OUTPUT);
  
  Serial.begin(9600);
  if (!LR.begin(KEYVAL)) 
    {Serial.println("No LoRa shield detected! It can't perform echo!");SHIELD=false;return;} 
  Serial.println("LoRa receiver."); 
 
  LR.defDevRange(DEVRANGE);
  LR.defNetAddress(LORANET);  

  Serial.print("Net address  : ");Serial.println(LORANET);
  Serial.print("Devices range: ");Serial.println(int(pow(2,DEVRANGE)-1));
  Serial.print("My address   : ");Serial.println(MYADD); 
  
  SX.setPower(PWR);
//  LR.setConfig(SF,BW,4); //if you want test different configuration (def: 9,6,4)
  showConfig();
  Serial.println("Waiting for message...");
  LR.receiveMessMode();   // set shield in continuous receiving mode
}

void loop() 
{
  int nb;
  if (!SHIELD) {delay(200);return;}
     // if mess. send echo and return back to receiving mode
  if ((nb=getMess())>0) {decode(nb);LR.receiveMessMode();} 
     // if push-button send unsolecited message and go back to receiving mode
  if (getInput()) {sendWarn();LR.receiveMessMode();}
}

int getMess()  // receive mess from master
{
  int nb;
  if ((nb=LR.receiveNetMess(MYADD,1,(byte*)recbuff,reclen))<=0) {delay(10);return 0;}
  Serial.print("< ");Serial.println(recbuff);
  snprintf(data,63,format,SX.getLoraRssi(),SX.lastLoraPacketRssi(),SX.lastLoraPacketSnr());
  Serial.println(data); 
  return nb;
}

void decode(int nb)
{
  char *mess=(char*)LR.getMessage();
  char rep[32];
  char *cmd;
  char *val;
  Serial.println(mess);
  cmd=strtok(mess,"=");
  val=strtok(NULL,"=");
  Serial.print(cmd);Serial.print(" ");Serial.println(val);
  if (strcmp(cmd,"echo")==0) {sendReply(val,strlen(val));return;}
  if (strcmp(cmd,"buzzer")==0)
   {actBuzzer(val);snprintf(rep,32,"OK Buzzer!");sendReply(rep,strlen(rep));return;}
  if (strcmp(cmd,"led")==0)
   {actLed(val);snprintf(rep,32,"OK Led %s !",val);sendReply(rep,strlen(rep));return;}
  if (strcmp(cmd,"getbutton")==0)
   {int b=getButt();snprintf(rep,32,"Button: %d !",b);sendReply(rep,strlen(rep));return;}
  snprintf(rep,32,"Unrec. cmd: %s",cmd);sendReply(rep,strlen(rep));
}

void sendReply(char *mess,int messlen)
{
  SX.setState(STDBY);              // stop receiving mode
  unsigned int dest=LR.getNetSender();
  Serial.print("Replay to ");Serial.println(dest); 
  delay(300);
  strlcpy(sendbuff,mess,messlen+1);
  Serial.print("> ");Serial.println(mess);
  if (LR.sendNetMess(dest,MYADD,(byte*)sendbuff,messlen)<0) Serial.println("Sending error!"); 
  else Serial.println("Sent...");
}

boolean getInput()
{
  if (digitalRead(pingo)>0) return false;
  int i;
  for (i=0;i<20;i++) {if (digitalRead(pingo)==0) break; delay(50);}
  return true;
}

void sendWarn()
{
  SX.setState(STDBY);              // stop receiving mode
  char warn[]="ATTENTION Button pressed!";
  Serial.print("> ");Serial.println(warn);
  if (LR.sendNetMess(MASTER,MYADD,(byte*)warn,strlen(warn)+1)<0) Serial.println("Sending error!"); 
  else Serial.println("Sent warn...");
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

void actBuzzer(char *val)
{
  char *pulse=strtok(val,",");
  char *rep=strtok(NULL,",");
  int p=atoi(pulse);
  int n=atoi(rep);
  sound(p,n);
}

void actLed(char *val)
{
  if (strcmp(val,"on")==0) digitalWrite(pinf,HIGH);
  if (strcmp(val,"off")==0) digitalWrite(pinf,LOW);
}

int getButt()
{
  return digitalRead(pingo);
}

void sound(int time,int n)
{
  int i;
  for (i=0;i<n;i++){digitalWrite(psound,1);delay(time);digitalWrite(psound,0);delay(100);}
  digitalWrite(psound,0);
}

