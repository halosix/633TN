/*  micronode 

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
      ATtiny85 pin 2 to voltage divider,
      use 10K for general, 1K for bright

*/

#include <DHT.h>
#include <Manchester.h>

int IDb = 140;                //change this to node ID
int beaconinterval = 10000;   //change this to desired beacon interval, in millis

int dhtpin = 0;
int txpin = 1;
int ldrpin = A1;

DHT dht(dhtpin, DHT11);

void setup() {
  dht.begin();
  man.setupTransmit(txpin, MAN_1200);
}

void loop() {
  int h = dht.readHumidity();
  int f = dht.readTemperature(true);
  int ldrr = analogRead(ldrpin);
  int ldr = map(ldrr, 0, 1023, 1, 100);
  man.transmit(253);
  delay(50);
  man.transmit(IDb);
  delay(50);
  man.transmit(h);
  delay(50);
  man.transmit(f);
  delay(50);
  man.transmit(ldr);
  delay(50);
  man.transmit(254);
  delay(beaconinterval);
}
