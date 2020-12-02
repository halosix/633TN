/*  micronode 

    v 2.1HTM
    02 DEC 20

    2908/6012 (48%) 64

    ported this micronode version to TinyDHT as well
    
    v 2.0HTM
    30 NOV 20

    this version is specifically for ATtiny85 with DHT11 and a single PIR sensor
    triggers when motion detected, sends a packet with humi + temp + motion code
    triggers again after (hardware-set) reset with humi + temp + end code
    
    v 1.2
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
      ATtiny85 pin 2 to voltage divider,
      use 10K for general, 1K for bright

*/
#include <TinyDHT.h>
#include <Manchester.h>

int IDb = 170;                // change this to node ID
int xmitspacing = 50;         // change this to desired intertransmission variable interval, in millis
long beaconinterval = 10000;  // change this to desired beacon interval, in millis

int dhtpin = 0;
int txpin = 1;
int inputPin = 2;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int val = LOW;                    // variable for reading the pin status

DHT dht(dhtpin, DHT11);

void setup() {
  pinMode(inputPin, INPUT_PULLUP);     // declare sensor as input
  dht.begin();
  man.setupTransmit(txpin, MAN_1200);
}

void loop() {

val = digitalRead(inputPin);


if (val == HIGH) {                    // triggered!  lets check the last state
  if (pirState == LOW) {              // its low, so this is a new trigger!  

  int h = dht.readHumidity();         // spin up the DHT and grab some data
  int f = dht.readTemperature(true);

  man.transmit(253);
  delay(xmitspacing);
  man.transmit(IDb);
  delay(xmitspacing);
  man.transmit(h);
  delay(xmitspacing);
  man.transmit(f);
  delay(xmitspacing);
  man.transmit(251);                  // 251 is the single-PIR activation code
  delay(xmitspacing);
  man.transmit(254);

  pirState = HIGH;                    // change last state to triggered
      }

  } else {                            // val is low, whats the last state
    if (pirState == HIGH){            // its high, we just reset.  send again
        
        int h = dht.readHumidity();   // spin up the DHT and grab some data
        int f = dht.readTemperature(true);
        
        man.transmit(253);
        delay(xmitspacing);
        man.transmit(IDb);
        delay(xmitspacing);
        man.transmit(h);
        delay(xmitspacing);
        man.transmit(f);
        delay(xmitspacing);
        man.transmit(252);            // 252 is the single-PIR deactivation code
        delay(xmitspacing);
        man.transmit(254);
      // We only want to print on the output change, not state
      pirState = LOW;
    }
  }
}
