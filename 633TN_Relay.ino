/*  633TN Relay 
    
    v 2.2
    30 NOV 20
    13238/28672 (46%) 822

    relay continued to be unstable, lasting max 2 days before requiring reset.
    changed if (i > 5) to if (i >= 5)
    reset i to 0 at sendframe(), was i=5
    
    v 2.1
    12 NOV 20
    13242/28672 (46%) 822

    relay was still hanging, seems to be missing 254 packets occasionally causing i to overrun
    added some basic error checking;
      changed (if i != 5) to (if i < 5)
      added (if i > 5)
      now clear the rxarray upon receipt of a 253
    added delay((random(0,4) * 50) prior to sending the LoRa frame in sendframe()
      im not positive collision detection is working in RF95
      this will allow multiple relays to operate hopefully without collisions
      if i determine CAD is working this can be removed

    
    v 2.01
    11 NOV 20
    13104/28672 (45#) 822

    relay tended to hang after approx 1h18m; changed i to 5 and cleared rxarray[] after transmitting frame
    moved LoRa pinmode declaration to setup because reasons
    
    v 2.0
    10 NOV 20
    
    12924/28672 (45%) 826
    Rewrote for much improved runtime stability, mostly by eliminating the use of while(!man.receiveComplete()) { delay(1); }
    
    
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

int stationID = 220;  // relay's station ID.  see project documentation for full addressing scheme
int i = 5;            // adjust for expected packet payload, +1 
int rxarray[3];       // adjust for expected payload

void setup() 
{
  randomSeed(analogRead(A0));     // lets grab some noise from A0 to seed the prng
  pinMode(RFM95_RST, OUTPUT);     // not strictly needed but good practice
  digitalWrite(RFM95_RST, HIGH);  // bring it high before we bounce it during init
  pinMode(A1, OUTPUT);      // these two lines are to bring the ASK receiver VCC high. 
  digitalWrite(A1, HIGH);   // this was done for ease of assembly, could put ASK RX to sleep if you wanted to

  delay(50);                // give the module a bit to power on
  
  man.setupReceive(RX_PIN, MAN_1200);   // initialize the ASK RX. we're using 1200 here, this can be changed
  man.beginReceive();                   // start listening to ASK RX

  Serial.begin(115200);
  Serial.println("633TN RELAY v2.01");
  
  rfinit();     // subroutine to initialize the RF95 chip

  delay(50);
}

void loop()
{
  if (man.receiveComplete())      // oh hey, we got a message! 
  {
    int m = man.getMessage();     // lets take it apart and stick it in m

    if (i < 5) {                 // i is our sequence flag, it's only 5 if we just started a fresh loop
      rxarray[i++] = m;
      Serial.print(m); Serial.print(",");
      man.beginReceive();
    }

    if (m == 253) {               // if the message is 253, that indicates a packet start. lets print that fact ...
      Serial.println(); Serial.print("Receiving packet .................."); Serial.print(m); Serial.print(",");
      i = 0;                      // and set our sequence flag to 0
      int rxarray[3];             // clear the array just in case
      man.beginReceive();         // start listening again!
    }    
    
    if (m == 254) {               // 254 means packet end, so we want to wrap things up and stuff it into a frame for retransmission
      sendframe();                // subroutine for sending a frame using the RF95
      man.beginReceive();         // start listening again!
    }        

    if (i >= 5) {                 // this should only happen if the ASK RX misses a 254 and keeps looping
      sendframe();                // so if this happens, send whatever is in the buffer and move on
      man.beginReceive();         
    }
    
  }
}

void rfinit()                     // this is basically straight from adafruit's RF95 FeatherWing code, i just stuck it here for neatness
{
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

void sendframe()      // we have a complete packet, and we are ready to encapsulate and send the frame
{
  i = 0;                                                                                                        // sequence flag back to 5
  char rframe[64];                                                                                              // start up our frame array
  sprintf(rframe, "633TN@255$%d;RL;%d,%d,%d,%d!", stationID, rxarray[0], rxarray[1], rxarray[2], rxarray[3]);   // assemble it
  Serial.println(); Serial.print("     Frame ready ... ["); Serial.print(rframe); Serial.println("]");          // print it to serial for funsies
  delay((random(0,4) * 50));                                                                                    // this just delays the frame tx by (50*rnd(0,4)) tx length is 125ms
  rf95.send((uint8_t *)rframe, strlen(rframe));                                                                 // send it to the RF95 module
  rf95.waitPacketSent();                                                                                        // let it do its thing
  int rxarray[3];                                                                                               // lets zero out that array
  Serial.println("Packet encapsulated and frame sent."); Serial.println("");                                    // all done! return to loop and start listening again
}
