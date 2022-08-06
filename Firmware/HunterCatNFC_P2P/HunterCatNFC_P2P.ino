/**
 * Example to detect P2P device 
 * Authors: 
 *        Salvador Mendoza - @Netxing - salmg.net
 *        For Electronic Cats - electroniccats.com
 * 
 *  March 2020
 * 
 * This code is beerware; if you see me (or any other collaborator 
 * member) at the local, and you've found our code helpful, 
 * please buy us a round!
 * Distributed as-is; no warranty is given.
 */

#include "Electroniccats_PN7150.h"   
#ifdef ARDUINO_ARCH_SAMD
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"
#endif
      
#ifdef ARDUINO_ARCH_SAMD
#define PN7150_IRQ   (15)
#define PN7150_VEN   (14)
#else
#define PN7150_IRQ   (18)
#define PN7150_VEN   (17)
#endif

#define PN7150_ADDR  (0x28)

#ifdef ARDUINO_ARCH_RP2040

#define PIN_LED  8
#define PIN_LED2 9
#define PIN_LED3 10

#define BUTTON_0 19
#define BUTTON_1 20
#define BUTTON_2 21
#endif

Electroniccats_PN7150 nfc(PN7150_IRQ, PN7150_VEN, PN7150_ADDR);    // creates a global NFC device interface object, attached to pins 7 (IRQ) and 8 (VEN) and using the default I2C address 0x28
RfIntf_t RfInterface;                                              //Intarface to save data for multiple tags

#ifdef ARDUINO_ARCH_SAMD
Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);

Adafruit_SPIFlash flash(&flashTransport);
#endif

uint8_t mode = 3;                                                  // modes: 1 = Reader/ Writer, 2 = Emulation, 3 = Peer to peer P2P

void ResetMode(){                                  //Reset the configuration mode after each reading
  Serial.println("Re-initializing...");
  nfc.ConfigMode(mode);                               
  nfc.StartDiscovery(mode);
}

void RGB(int R, int G, int B)
{
  int g = map(G, 0, 255, 0, 1023);

  analogWrite(PIN_LED, R);
  analogWrite(PIN_LED2, B);
  analogWrite(PIN_LED3, g);
}

void setup(){
  Serial.begin(9600);
  while(!Serial);

  #ifdef ARDUINO_ARCH_SAMD
  // Initialize flash library and check its chip ID.
  if (!flash.begin()) {
    Serial.println("Error, failed to initialize flash chip!");
    while (1) {
      blink(LED_BUILTIN, 600, 3);;
    }
  }
  Serial.print("Flash chip JEDEC ID: 0x"); Serial.println(flash.getJEDECID(), HEX);
#endif

  Serial.println("Initializing...");                
  if (nfc.connectNCI()) { //Wake up the board
    Serial.println("Error while setting up the mode, check connections!");
    while (1);
  }
  
  if (nfc.ConfigureSettings()) {
    Serial.println("The Configure Settings failed!");
    while (1);
  }
  
  if(nfc.ConfigMode(mode)){ //Set up the configuration mode
    Serial.println("The Configure Mode failed!!");
    while (1);
  }
  nfc.StartDiscovery(mode); //NCI Discovery mode
  Serial.println("Detect P2P devices with PN7150");
  Serial.println("Waiting for P2P device...");
}

void loop(){
  int analog = analogRead(A0);
  int voltagepercent = map(analog, 0, 645, 0, 100 );

 
   
  if(!nfc.WaitForDiscoveryNotification(&RfInterface)){ // Waiting to detect 
   if (RfInterface.Interface == INTF_NFCDEP){
    if ((RfInterface.ModeTech & MODE_LISTEN) == MODE_LISTEN){ 
     //Serial.println(" - P2P TARGET MODE: Activated from remote Initiator");
     Serial.println("Listening ");
     RGB(255,0,0);
    }
    else{
     //Serial.println(" - P2P INITIATOR MODE: Remote Target activated");
     Serial.println("Talking ");
     RGB(0,0,255);
    }
   /* Process with SNEP for NDEF exchange */
   nfc.ProcessP2pMode(RfInterface);
   }
  ResetMode();
  RGB(0,0,0); 
  Serial.println("Peer lost!"); 
 }
   
  delay(500);

   //Low Battery < 30%
   if (voltagepercent < 30) {
   RGB(234, 200, 203); //Pink
   delay(100);
   RGB(0, 0, 0);
   delay(100);
   }
}
