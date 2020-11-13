/*  633TN Gateway

    v 2.0m
    13 NOV 20
    58248/262144 (22%)

    changed decision tree, now if (IDs >= xxx && IDs <=yyy) to determine how to handle packet payload 
    
    
    v 1.1m
    10 NOV 20

    41480/262144 (15%) 
    removed OLED display, compiles for m0 WINC 1500 wifi, plus RM95 breakout
    
    v 1.0
    08 NOV 20
    23298/28672 (81%), 1177 bytes global vars

    This is an Adafruit 32u4 Feather with RF95 915MHz LoRa onboard,
    with an Adafruit OLED FeatherWing attached

    This will be ported to an M0 Feather ASAP for much greater utility

    This device monitors the 915 MHz ISM band (user-configurable frequency) and upon receipt of a valid
    frame, decodes it and presents the information on the display

    Serial output is enabled for debugging or logging purposes, but is not required

    For network details, see documentation on https://github.com/halosix/633TN

*/

#include <SPI.h>
#include <Wire.h>
#include <RH_RF95.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <WiFi101.h>

#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2     // or, tie EN to VCC

char ssid[] = "ssid";     //  your network SSID (name)
char pass[] = "password";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

#define MQTT_SERVER      "10.10.10.10"
#define MQTT_SERVERPORT  1883
#define MQTT_USERNAME    "username"
#define MQTT_KEY         "key"

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_KEY);

#define halt(s) { Serial.println(F( s )); while(1);  }

// feed setup

Adafruit_MQTT_Publish pubtest = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/test");


#define LED      13
#define RF95_FREQ 915.0
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     6    // "D"

RH_RF95 rf95(RFM95_CS, RFM95_INT);

int myaddress = 200;                // gateway station ID, see project documentation for full addressing scheme

String network = "";
int rxdest = 0;
int whofrom = 0;
String ftype = "";
int IDs = 0;
int Hs = 0;
int Ts = 0;
int Ls = 0;

uint32_t x=0;

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);
  

  Serial.begin(115200);

  delay(100);

  rfinit();

    // Initialise the Client
  Serial.print(F("\nInit the WiFi module..."));
  // check for the presence of the breakout
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WINC1500 not present");
    // don't continue:
    while (true);
  }
  Serial.println("ATWINC OK!");
}

void loop() 
{

  MQTT_connect();
  
  if (rf95.available()) {

    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {

      digitalWrite(LED, HIGH);
      Serial.println(); Serial.println("FRAME RECEIVED");
      Serial.print("Got: ");
      Serial.println((char*)buf);

      buf[len] = 0;

      network = strtok((char*)buf, "@");  // first we look at the frame -  return data to the left of it as a String, passing the remainder to the next strtok()
      rxdest = atoi(strtok(NULL, "$"));   // parses out intended recipient ID
      whofrom = atoi(strtok(NULL, ";"));  // parses out sender ID
      ftype = strtok(NULL, ";");          // parses out frame type
      IDs = atoi(strtok(NULL, ","));      // parses out remote sender ID

      if (network = "633TN") {            // ...now, IF the network type is 633TN ...

        sixthreethreetn();                // go here!

      }

      // else if (network = "xxxxx") { }    // other networks can go here

    }

  }

}

void rfinit() 
{
  Serial.println("Feather LoRa RX Test!");

  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  rf95.setTxPower(23, false);
}

void sixthreethreetn()
{
  if (rxdest = 255) {      // and IF the destination is Broadcast-All ...

  if (ftype = "RL") {       // and IF the frame type is Relay ...

  if (IDs >= 140 && IDs <=149) {       // and IF the remote sender is in the 140s, we know its a DHT11 and LDR ... so do all this stuff to it.

  Hs = atoi(strtok(NULL, ","));  // parses out humidity
  Ts = atoi(strtok(NULL, ","));  // parses out temperature
  Ls = atoi(strtok(NULL, "!"));  // parses out light level

  pubtest.publish(x++);
  
  Serial.print("Network Type: "); Serial.println(network);
  Serial.print("Sender: "); Serial.println(whofrom);
  Serial.print("Frame Type: "); Serial.println(ftype);
  Serial.print("Remote sender: "); Serial.println(IDs);
  Serial.print("Humidity: "); Serial.println(Hs);
  Serial.print("Temperature: "); Serial.println(Ts);
  Serial.print("Light level: "); Serial.println(Ls);
  Serial.print("RSSI: "); Serial.println(rf95.lastRssi(), DEC);
  Serial.println("");
  Serial.println();
  }

  // else if (IDs >= xxx && IDs <=yyy) { }      // other node addresses, future use, maybe motion detectors
            
  }
  
  // else if (ftype = "XX") { }                 // other frame types can go here

  }

  // else if (rxdest = xxx) { }                 // other rxdests can go here

}

void MQTT_connect() 
{
  int8_t ret;

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    uint8_t timeout = 10;
    while (timeout && (WiFi.status() != WL_CONNECTED)) {
      timeout--;
      delay(1000);
    }
  }
  
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}
