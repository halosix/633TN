/*  micronode 
    
    v1.21
    09 NOV 20
    4956/6012 (82%) 112

    25ms seems best for relay stability; 10ms worked fine for short periods then locked
    beaconinterval changed to 60 seconds default
    changed variable names to more generic ones; this is preparatory to modularizing
    the code to accomodate future sensor types
    
    v1.2
    08 NOV 20
    4730/6012 80
    decreased transmission interval from 50ms to 10ms, still stable in operation
    will monitor, may revert to 25ms if any problems crop up
    shorter interval means relay spends less time waiting for complete message,
    and airtime efficiency is increased
    moved xmitspacing to int() variable, easier tweaking
    changed beaconinterval from int() to long() to allow for greater intervals
    
    v 1.1
    08 NOV 20
    4678/6012, 76 bytes global vars
    streamlined
    changed delay to variable, easier to change
    moved node ID for ease of use
    hardcoded start and end numbers, no need for these to be variables
    
    v 1.0
    31 OCT 20
    4644/6012 bytes, 74 bytes global variables
    ATtiny85, 433Mhz ASK TX, DHT11, and LDR
      ATtiny85 pin 0 to DHT11 OUT
      ATtiny85 pin 1 to ASK TX DATA
      ATtiny85 pin 2 to LDR voltage divider,
      use 10K for general, 1K for bright
*/

#include <DHT.h>
#include <Manchester.h>

int IDb = 140;                // change this to node ID
int xmitspacing = 25;         // change this to desired intertransmission variable interval, in millis
long beaconinterval = 60000;  // change this to desired beacon interval, in millis

int dhtpin = 0;
int txpin = 1;
int ldrpin = A1;

DHT dht(dhtpin, DHT11);

void setup() {
  pinMode(ldrpin, INPUT);
  dht.begin();
  man.setupTransmit(txpin, MAN_1200);
}

void loop() {
  int h = dht.readHumidity();
  int f = dht.readTemperature(true);
  int ldrr = analogRead(ldrpin);
  int ldr = map(ldrr, 0, 1023, 1, 100);
  man.transmit(253);
  delay(xmitspacing);
  man.transmit(IDb);
  delay(xmitspacing);
  man.transmit(h);
  delay(xmitspacing);
  man.transmit(f);
  delay(xmitspacing);
  man.transmit(ldr);
  delay(xmitspacing);
  man.transmit(254);
  delay(beaconinterval);
}
