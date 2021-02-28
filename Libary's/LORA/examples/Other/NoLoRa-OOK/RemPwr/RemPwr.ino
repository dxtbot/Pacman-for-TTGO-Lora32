/* Remote 220V switch on/off for Avidsen or Velleman. It uses SX1278 system. */
/* Optimized for RandA. It replayes always with "OK" or "ON" or "OF".                                                      */
/* 
*  Command (serial 9600):
*     ? : return name (itsMe function) (reply name)
*     onx : switch on;  where x=1,2,3,4,5  (1,2,3 for Velleman) (reply "ON")
*     ofx : switch off; where x=1,3,3,4,5  (1,2,3 for Velleman) (reply "OF")
*     av  : set for Avidsen system (active for next command)(default) (reply "OK")
*     ve  : set for Velleman system (active for next command) (reply "OK")
*     adxxxxxxx : set address (three state ex.: 0200220) (just 5 char for Avidsen) (reply "OK")
*     pwx  : set transmit power x=1(7dBm),2(10dBm),3(13dBm),4(17dBm)(def),5(20dBm)
*
*  ex.: 
*      ve
*      ad00222
*      on2
* 
* Author: D. Denaro
******************************************************************************/

#include "SX1278.h"
#include "REMOTEC.h"
#include <SPI.h>

#define ln 10


REMOTEC RC;

byte addAv[5]={2,0,2,0,0};int alenAv=5;
byte addVl[7]={2,0,2,0,2,2,2};int alenVl=7;
byte* add=addAv;
int alen=alenAv;

int mode=1;

char buff[32];

byte sock;
byte onoff;

void setup() 
{
  Serial.begin(9600);
  RC.begin();
  setmodeavidsen();
}

void loop() 
{
  int c=Serial.available();
  if(c>0) 
  {
   Serial.readBytesUntil(ln,buff,32);
   if (buff[0]=='?') itsMe();
   if (strncasecmp(buff,"on",2)==0)  commandSend(1);
   if (strncasecmp(buff,"of",2)==0)  commandSend(0);
   if (strncasecmp(buff,"av",2)==0)  setmodeavidsen();
   if (strncasecmp(buff,"vl",2)==0)  setmodevelleman();
   if (strncasecmp(buff,"ad",2)==0)  setaddress();
   if (strncasecmp(buff,"pw",2)==0)  setpower();
  }
}

void itsMe()
{
  char name[]=__FILE__;
  char* c=strchr(name,'.');
  if (c != NULL) *c='\0';
  Serial.println(name);
}

void setmodeavidsen()
{
  mode=1;
  add=addAv;
  alen=alenAv;
  Serial.println("OK");
}

void setmodevelleman()
{
  mode=2;
  add=addVl;
  alen=alenVl;
  Serial.println("OK");
}

void commandSend(byte onoff)
{
  byte sock=atoi(&buff[2]);
  if (mode==1)  RC.avidsenSet(add,sock,onoff);
  if (mode==2)  RC.vellemanSet(add,sock,onoff);
  if (onoff==1)Serial.println(" ON"); else Serial.println(" OF");
}

void setaddress()
{
  int i;
  if (mode==1){for(i=2;i<7;i++) addAv[i-2]=buff[i]-'0';}  
  if (mode==2){for(i=2;i<9;i++) addVl[i-2]=buff[i]-'0';}
  Serial.println("OK");
}

void setpower()
{
  byte power=atoi(&buff[2]);
  if ((power<1)|(power>5)){Serial.println("NK");return;}
  SX.setPower(power);
  Serial.println("OK");
}

