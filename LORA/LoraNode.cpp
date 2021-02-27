
#include "LoraNode.h"
#include <EEPROM.h>

void LoraNode::initDefault()
{
  NETADD=defNETADD;
  NUMDEVCODE=defNUMDEVCODE;
  NODEADD=defNODEADD;
  KEYVAL=defKEYVAL;

  FREQ=defFREQ;

  SF=defSF;
  BW=defBW;
  CR=defCR;
  PWR=defPWR; 
  
  factive=false;
  
  bufflen=receiveBufferLen+6;
  recbuff=new byte[bufflen];
  
  LR.defDevRange(NUMDEVCODE);         
  LR.defNetAddress(NETADD); 

  autoAK=false; 
}

LoraNode::LoraNode()
{
  initDefault();
  byte b0=EEPROM.read(0);
  byte b1=EEPROM.read(1);
  if (b1==255) return; 
  unsigned int nodeAdd=word(b1,b0);
  setNodeAdd(nodeAdd);  
}  

LoraNode::LoraNode(unsigned int nodeAdd)
{
  initDefault();
  setNodeAdd(nodeAdd);
}

void LoraNode::setNodeAdd(unsigned int nodeAdd)
{
  unsigned int maxd=getMaxDevices();
  if (nodeAdd<1) nodeAdd=1;
  if (nodeAdd>maxd) nodeAdd=maxd;
  NODEADD=nodeAdd;
}

void LoraNode::saveNodeAdd()
{
  EEPROM.write(0,lowByte(NODEADD));
  EEPROM.write(1,highByte(NODEADD));
}

unsigned int LoraNode::getNodeAdd() {return NODEADD;}

bool LoraNode::begin()
{
  if (!LR.begin(KEYVAL)) return false; 
  factive=true;
  LR.setFrequency(FREQ);  
  LR.setPower(PWR);
  LR.setConfig(SF,BW,CR); 
  return true; 
}

void LoraNode::setKey(int key){KEYVAL=key;}

void LoraNode::setMaxDevices(byte code)
{
  if (code<2) code=2;if (code>14) code=14;
  NUMDEVCODE=code;
  unsigned int maxid=LR.defDevRange(NUMDEVCODE);
  if (NETADD>maxid) NETADD=maxid; 
}
     
unsigned int LoraNode::getMaxDevices(){return pow(2,NUMDEVCODE);}      

void LoraNode::setNetId(unsigned int id)
{
  unsigned int maxid=LR.getMaxNetAddress();
  if (id>maxid) id=maxid;
  NETADD=id;
  LR.defNetAddress(NETADD);
}

unsigned int LoraNode::getNetId(){ return NETADD;}

unsigned int LoraNode::getMaxId(){return LR.getMaxNetAddress();}

void LoraNode::setFrequency(float nMhz) {FREQ=nMhz;if (factive) SX.setFreq(FREQ);}
float LoraNode::getFrequency(){if (factive)return SX.readFreq();else return FREQ;}

void LoraNode::setSpreadingFactor(byte code){SF=code;if (factive)SX.setLoraSprFactor(SF);}

void LoraNode::setBandWidth(byte code){BW=code;if (factive)SX.setLoraBw(BW);}

void LoraNode::setPower(byte code){PWR=code;if (factive)SX.setPower(PWR);}
int LoraNode::getPowerCode(){if (factive)return (int)SX.getPower(0);else return PWR;}

void LoraNode::printConfig()
{
  Serial.print("LoRa module version: ");char v[4];Serial.println(SX.readVersion(v));
  Serial.print("Frequence: ");Serial.println(SX.readFreq()); 
  Serial.print("Transmit power (mW): ");Serial.println(SX.getPower(2)); 
  Serial.print("Preamble bytes: ");Serial.print(SX.getLoraPreambleLen());Serial.println("+4"); 
  Serial.print("SpFactor: ");Serial.println(SX.getLoraSprFactor());
  Serial.print("BandWcode: ");Serial.print(SX.getLoraBw());Serial.print(" -> kHz ");Serial.println(SX.getLoraBwFreq()); 
  Serial.print("Cr_code: ");Serial.println(SX.getLoraCr());
  Serial.print("Rate (bps): ");Serial.print(SX.getLorabps());
  Serial.print(" Byte/sec: ");Serial.println(SX.getLorabps()/8);  
}

void LoraNode::printAddresses()
{
  Serial.print("Net address range: 0 to ");Serial.print(0xFFFF>>NUMDEVCODE);
  Serial.print("  Devices range: 1 to ");Serial.println((int)pow(2,NUMDEVCODE)-1);
  Serial.print("Net address   : ");Serial.println(NETADD);
  Serial.print("My address    : ");Serial.println(NODEADD); 
}

void LoraNode::changeMessageBufferLen(int maxtextlen)
{
   bufflen=maxtextlen+6;
   delete recbuff;
   recbuff=new char[bufflen];
}

void LoraNode::setAutomaticAck(bool set){autoAK=set;}

bool LoraNode::writeMessage(int dest,char* message,int timeout)
{
  int i;
  for (i=0;i<timeout;i++) {if (LR.freeAir()) break;else delay(1);}
  if (i>=timeout) {return false;}
  if (LR.sendNetMess(dest,NODEADD,message)<0) {return false;}
  if (dest==0) return true; 
  if (!autoAK) return true;
  int nc=LR.receiveNextMessage(NODEADD,dest,recbuff,bufflen,200);
  if (nc==0) {return false;}
  char* ack=LR.getMessage();
  if (strncmp(ack,"AK",2)!=0) {return false;}
  return true;
}

bool LoraNode::newMessAvailable(int timeout)
{return incomingMessage(0,timeout);}

bool LoraNode::newMessAvailable(int from,int timeout)
{return incomingMessage(from,timeout);}

char* LoraNode::getMessage(){return LR.getMessage();}
int  LoraNode::getSender(){return LR.getSender();}
int  LoraNode::getMarker(){return LR.getMarker();}
  
bool LoraNode::incomingMessage(int from,int timeout)
{
  int nc=LR.receiveNextMessage(NODEADD,from,recbuff,bufflen,timeout);
  if (nc<=0) return false;
  if (!autoAK) return true;
  int sender=LR.getSender();
  int i;for (i=0;i<300;i++) {if (LR.freeAir()) break;else delay(1);}
  if (i>=300) {return false;}
  if (LR.sendNetMess(sender,NODEADD,"AK")<0)   {return false;}
  return true;
}

/************** Expansion for binary data ***************/
bool LoraNode::writeMessageByte(int dest,byte message[],int messlen,int timeout)
{
  int i;
  for (i=0;i<timeout;i++) {if (LR.freeAir()) break;else delay(1);}
  if (i>=timeout) {return false;}
  if (LR.sendNetMess(dest,NODEADD,message)<0) {return false;}
  if (dest==0) return true; 
  if (!autoAK) return true;
  int nc=LR.receiveNextMessage(NODEADD,dest,recbuff,bufflen,200);
  if (nc==0) {return false;}
  char* ack=LR.getMessage();
  if (strncmp(ack,"AK",2)!=0) {return false;}
  return true;
}

bool LoraNode::newMessByteAvailable(int from,byte binbuff[],int bufflen,int timeout)
{
  int nc=LR.receiveNextMessage(NODEADD,from,binbuff,bufflen,timeout);
  if (nc<=0) return false;
  if (!autoAK) return true;
  int sender=LR.getSender();
  int i;for (i=0;i<300;i++) {if (LR.freeAir()) break;else delay(1);}
  if (i>=300) {return false;}
  if (LR.sendNetMess(sender,NODEADD,"AK")<0)   {return false;}
  return true;
}

byte* LoraNode::getMessageByte(){return LR.getMessage();}

int LoraNode::getNumByteReceived() {return LR.getReceivedMessLen();}

/********************************************************/

bool LoraNode::freeAir(){return LR.freeAir();}

void LoraNode::loadNetConfig()
{
  byte b0;
  byte b1;
  b0=EEPROM.read(4);
  if ((b0<13)&(b0>2)) setMaxDevices(b0);
  b0=EEPROM.read(0);
  b1=EEPROM.read(1);
  if (b1<=0x20){unsigned int nodeAdd=word(b1,b0);setNodeAdd(nodeAdd);}
  b0=EEPROM.read(2);
  b1=EEPROM.read(3);
  if (b1<=0x20) {unsigned int id=word(b1,b0);setNetId(id);} 
  b0=EEPROM.read(5);
  b1=EEPROM.read(6);
  if (!((b0==255)&(b1==255))) {unsigned int key=word(b1,b0);setKey(key);}
}

void LoraNode::saveNetConfig()
{
  EEPROM.write(0,lowByte(NODEADD));
  EEPROM.write(1,highByte(NODEADD));
  EEPROM.write(2,lowByte(NETADD));
  EEPROM.write(3,highByte(NETADD));
  EEPROM.write(4,lowByte(NUMDEVCODE));
  EEPROM.write(5,lowByte(KEYVAL));
  EEPROM.write(6,highByte(KEYVAL));
}

void LoraNode::loadRadioConfig()
{
  byte* fb=(byte*)&FREQ;
  byte b[4];
  for (int i=0;i<4;i++) {b[i]=EEPROM.read(i+7);*(fb+i)=b[i];}
  SF=EEPROM.read(11);
  BW=EEPROM.read(12);
  PWR=EEPROM.read(13);
}

void LoraNode::saveRadioConfig()
{
  byte* fb=(byte*)&FREQ;
  byte b[4];
  for (int i=0;i<4;i++) {b[i]=*(fb+i);EEPROM.write(i+7,b[i]);}
  EEPROM.write(11,SF);
  EEPROM.write(12,BW);
  EEPROM.write(13,PWR);
  
}

void LoraNode::resetEEPROM()
{
  for (int i=0;i<14;i++) EEPROM.write(i,255);
}