/*  633TN Relay 
    
    v 1.0
    08 NOV 20

    12284/28672 (42%), 803 bytes global vars

    This is an Adafruit 32u4 Feather with RF95 915MHz LoRa onboard
    There is a generic 434 MHz ASK receiver, vcc on A1 and data on A2

    This device monitors 434 MHz, and upon receipt of a valid Manchester-encoded signal, 
    assembles the received data into a frame which it then rebroadcasts on 915 MHz LoRa

    Serial output is enabled for debugging or logging purposes, but is not required

    For network details, see documentation on https://github.com/halosix/633TN

*/

#include <Manchester.h>
#include <SPI.h>
#include <RH_RF95.h>

#define RX_PIN A2
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

#define RF95_FREQ 915.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

int stationID = 220;
int IDs = 0;
int Hs = 0;
int Ts = 0;
int Ls = 0;

void setup() 
{
  pinMode(A1, OUTPUT);    // these two lines are to bring the ASK receiver VCC high. this was done for ease of assembly but could also be modified to put the ASK RX to sleep
  digitalWrite(A1, HIGH);
  
  man.setupReceive(RX_PIN, MAN_1200);
  man.beginReceive();

  Serial.begin(115200);
  
  delay(50);
  
  Serial.println("633TN RELAY TEST");
  
  rfinit();
}

void loop()
{
  if (man.receiveComplete()) 
  {
    int m = man.getMessage();
    
    if (m == 253) {
      Serial.print("Receiving packet ... [                 ");
      man.beginReceive();
      
      while(!man.receiveComplete()) { delay(1); }
      IDs = man.getMessage();
      Serial.print(IDs); Serial.print(",");
      man.beginReceive();
      
      while(!man.receiveComplete()) { delay(1); }
      Hs = man.getMessage();
      Serial.print(Hs); Serial.print(",");
      man.beginReceive();
      
      while(!man.receiveComplete()) { delay(1); }
      Ts = man.getMessage();
      Serial.print(Ts); Serial.print(",");
      man.beginReceive();
      
      while(!man.receiveComplete()) { delay(1); }
      Ls = man.getMessage();
      Serial.print(Ls); Serial.println("!]");
      man.beginReceive();   

      while(!man.receiveComplete()) { delay(1); }

      if (man.getMessage() == 254) {
      char rpacket[64];
      sprintf(rpacket, "633TN@255$%d;RL;%d,%d,%d,%d!", stationID, IDs, Hs, Ts, Ls);
      Serial.print("     Frame ready ... ["); Serial.print(rpacket); Serial.println("]");
      rf95.send((uint8_t *)rpacket, strlen(rpacket));
      rf95.waitPacketSent();
      Serial.println("Packet encapsulated and frame sent."); Serial.println("");
      man.beginReceive();       
      }
    }
  }
}

void rfinit() {

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }

  rf95.setTxPower(23, false);
  
}
