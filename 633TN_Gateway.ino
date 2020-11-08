/*  633TN Gateway 
    
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
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 oled = Adafruit_SSD1306();

#define BUTTON_A 9
#define BUTTON_B 6
#define BUTTON_C 5
#define LED      13

#define RFM95_CS      8
#define RFM95_INT     7
#define RFM95_RST     4

#define RF95_FREQ 915.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

String network = "";
int rxdest = 0;
int whofrom = 0;
String ftype = "";
int IDs = 0;
int Hs = 0;
int Ts = 0;
int Ls = 0;
    

void setup() {
  delay(500);
  Serial.begin(115200);
  Serial.println("633TN Gateway v1.0b");
  
  pinMode(LED, OUTPUT);     
  
  rfinit();
  oledinit();
}

void loop() {
  if (rf95.available()) {

    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
    
    digitalWrite(LED, HIGH);
    Serial.print("Got: ");
    Serial.println((char*)buf);

    buf[len] = 0;
    
    network = strtok((char*)buf, "@");  // first we look at the frame -  return data to the left of it as a String, passing the remainder to the next strtok()
    rxdest = atoi(strtok(NULL, "$"));  // parses out intended recipient ID
    whofrom = atoi(strtok(NULL, ";"));  // parses out sender ID
    ftype = strtok(NULL, ";");  // parses out frame type

    if (network = "633TN") {     // ...now, IF the network type is 633TN ...
    
      if (rxdest = 255) {      // and IF the destination is Broadcast-All ... 
      
        if (ftype = "RL") {       // and IF the frame type is Relay ... do all this stuff to it.
        
          IDs = atoi(strtok(NULL, ","));  // parses out remote sender ID
          Hs = atoi(strtok(NULL, ","));  // parses out humidity
          Ts = atoi(strtok(NULL, ","));  // parses out temperature
          Ls = atoi(strtok(NULL, "!"));  // parses out light level

          Serial.println("FRAME RECEIVED");
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
        
        // put other packet types here
      
      }
    
      // put other rx destinations here
    
    }
    
    // put other networks here
    
    }

    // may as well draw on the screen now that we have the packet

    oled.clearDisplay();
    oled.setCursor(0,0);
    oled.print("Network: "); oled.println(network);
    oled.print("From "); oled.print(IDs); oled.print(" via "); oled.println(whofrom);
    oled.print("Payload "); oled.print(Hs); oled.print(" "); oled.print(Ts); oled.print(" "); oled.println(Ls); 
    oled.print("RSSI: "); oled.println(rf95.lastRssi());
    oled.display(); 
  } 
  
}

void oledinit() {

oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
oled.display();
delay(250);
oled.clearDisplay();
oled.display();
oled.setTextSize(1);
oled.setTextColor(WHITE);
oled.setCursor(0,0);
oled.println("633 Telemetry Network");
oled.println("");
oled.println("633TN_Gateway v1.0b");
oled.println("halosix technologies");
oled.display();

delay(250);

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
