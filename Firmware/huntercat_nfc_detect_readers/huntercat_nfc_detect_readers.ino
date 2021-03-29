/**
   Example to detect NFC readers.

   Authors:
          Salvador Mendoza - @Netxing - salmg.net
          For Electronic Cats - electroniccats.com

   March 2021

   This code is beerware; if you see me (or any other collaborator
   member) at the local, and you've found our code helpful,
   please buy us a round!
   Distributed as-is; no warranty is given.

   ---
   
   This code will detect NFC card readers.

   When it starts, the three LEDs will be on, and then will turn off after a second.
   Then a constant LED 1 will start flashing meaning that is scanning for NFC readers.
   When a NFC reader is detected, LED 2 and 3 will be on for a second.
   
*/

#include "Electroniccats_PN7150.h"
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"

#define PN7150_IRQ   (15)
#define PN7150_VEN   (14)
#define PN7150_ADDR  (0x28)

Electroniccats_PN7150 nfc(PN7150_IRQ, PN7150_VEN, PN7150_ADDR);  // creates a global NFC device interface object, attached to pins 7 (IRQ) and 8 (VEN) and using the default I2C address 0x28
RfIntf_t RfInterface;                                            //Intarface to save data for multiple tags

Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);

Adafruit_SPIFlash flash(&flashTransport);

unsigned char STATUSOK[] = {0x90, 0x00}, Cmd[256], CmdSize;

uint8_t mode = 2;  // modes: 1 = Reader/ Writer, 2 = Emulation

void RGB(int R, int G, int B)
{
  int g = map(G, 0, 255, 0, 1023);
  
  analogWrite(PIN_LED,R);
  analogWrite(PIN_LED2,B);
  analogWrite(PIN_LED3,g);
}

void blink(int pin, int msdelay, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(msdelay);
    digitalWrite(pin, LOW);
    delay(msdelay);
  }
}

void setup() {
  Serial.begin(9600);
  //while (!Serial);
  Serial.println("Detecting NFC readers with PN7150");
  
  pinMode(LED_BUILTIN, OUTPUT); //RED
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED3, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PIN_LED2, LOW);
  digitalWrite(PIN_LED3, LOW);

  // Initialize flash library and check its chip ID.
  if (!flash.begin()) {
    Serial.println("Error, failed to initialize flash chip!");
    while(1){
      blink(LED_BUILTIN, 600, 3);;
    }
  }
  Serial.print("Flash chip JEDEC ID: 0x"); Serial.println(flash.getJEDECID(), HEX);

  Serial.println("Initializing...");
  if (nfc.connectNCI()) { //Wake up the board
    Serial.println("Error while setting up the mode, check connections!");
    while (1){
      blink(LED_BUILTIN, 200, 3);
    }
  }

  if (nfc.ConfigureSettings()) {
    Serial.println("The Configure Settings failed!");
    while (1){
      blink(LED_BUILTIN, 200, 5);
    }
  }

  if (nfc.ConfigMode(mode)) { //Set up the configuration mode
    Serial.println("The Configure Mode failed!!");
    while (1){
      blink(LED_BUILTIN, 200, 10);
    }
  }
  nfc.StartDiscovery(mode); //NCI Discovery mode
  blink(LED_BUILTIN, 200, 2);
  blink(PIN_LED2, 200, 2);
  blink(PIN_LED3, 200, 2);
  Serial.println("HunterCat NFC v1.3");
  Serial.println("Looking for card readers...");
}

// to detect NFC card readers
void loop() {
  
  if (nfc.CardModeReceive(Cmd, &CmdSize) == 0) { //Data in buffer?
    if ((CmdSize >= 2) && (Cmd[0] == 0x00)) { //Expect at least two bytes
      switch (Cmd[1]) {
        case 0xA4: //If tries to select a file, meaning that it is a reader
          Serial.println("Card reader detected!");
          digitalWrite(PIN_LED2, HIGH);
          digitalWrite(PIN_LED3, HIGH);
          delay(1000);
          digitalWrite(PIN_LED2, LOW);
          digitalWrite(PIN_LED3, LOW);
          break;

        default:
          break;
      }
      nfc.CardModeSend(STATUSOK, sizeof(STATUSOK));
    }
  }
  
  int analog = analogRead(A0);
  int voltagepercent = map(analog, 0, 645, 0, 100 );
  
  //Low Battery < 30%
  if(voltagepercent < 30){
    RGB(234, 200, 203); //Pink
    delay(100);
    RGB(0, 0, 0);
    delay(100);
  }
  
  blink(LED_BUILTIN, 10, 1);
}
