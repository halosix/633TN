/*  micronode v 1.0
    pleione
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


int dhtpin = 0;
int txpin = 1;
int ldrpin = A1;

byte IDb = 140;
byte IDx = 254;

DHT dht(dhtpin, DHT11);

void setup() {
  dht.begin();
  man.setupTransmit(txpin, MAN_1200);
}

void loop() {

  float h = dht.readHumidity();
  float f = dht.readTemperature(true);
  int ldrr = analogRead(ldrpin);
  int ldr = map(ldrr, 0, 1023, 10, 250);
  
  man.transmit(IDb);
  delay(10);
  man.transmit(h);
  delay(10);
  man.transmit(f);
  delay(10);
  man.transmit(ldr);
  delay(10);
  man.transmit(IDx);
  
  delay(10000);
}
