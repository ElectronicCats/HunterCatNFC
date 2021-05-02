/**
 * Example detect & read ntag amiibo 
 * Authors: 
 *        Salvador Mendoza - @Netxing - salmg.net
 *        For Electronic Cats - electroniccats.com
 * 
 *  April 2021
 * 
 * This code is beerware; if you see me (or any other collaborator 
 * member) at the local, and you've found our code helpful, 
 * please buy us a round!
 * Distributed as-is; no warranty is given.
 */

#include "Electroniccats_PN7150.h"          
#define PN7150_IRQ   (15)
#define PN7150_VEN   (14)
#define PN7150_ADDR  (0x28)

Electroniccats_PN7150 nfc(PN7150_IRQ, PN7150_VEN, PN7150_ADDR);    // creates a global NFC device interface object, attached to pins 7 (IRQ) and 8 (VEN) and using the default I2C address 0x28
RfIntf_t RfInterface;                                              //Intarface to save data for multiple tags

uint8_t mode = 1;                                                  // modes: 1 = Reader/ Writer, 2 = Emulation

int ResetMode(){                                  //Reset the configuration mode after each reading
  Serial.println("Re-initializing...");
  nfc.ConfigMode(mode);                               
  nfc.StartDiscovery(mode);
}

void PrintBuf(const byte * data, const uint32_t numBytes){ //Print hex data buffer in format
  uint32_t szPos;
  for (szPos=0; szPos < numBytes; szPos++)
  {
    Serial.print(F("0x"));
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      Serial.print(F("0"));
    Serial.print(data[szPos]&0xff, HEX);
    if ((numBytes > 1) && (szPos != numBytes - 1))
    {
      Serial.print(F(" "));
    }
  }
  Serial.println();
}
void readntag(){
  bool status;
  unsigned char Resp[256];
  unsigned char RespSize;  
  
  for (uint8_t i = 0; i < 42; i++) {
      unsigned char ReadBlock[] = {0x30, i};

      status = nfc.ReaderTagCmd(ReadBlock, sizeof(ReadBlock), Resp, &RespSize);
      
      // Display the current page number
      Serial.print("Page ");Serial.print(i);
      Serial.print(": ");
  
      // Display the results, depending on 'success'
      if (status == 0) {
        PrintBuf(Resp, 4);
      }
      else {
        Serial.println("Unable to read the requested page!");
      }
    }      
}

void displayCardInfo(RfIntf_t RfIntf){ //Funtion in charge to show the card/s in te field
  char tmp[16];
  while (1){
    switch(RfIntf.Protocol){  //Indetify card protocol
      case PROT_T2T:
          Serial.print(" - POLL MODE: PROT_T2T ");
          break;
      default:
          Serial.println(" - POLL MODE: Undetermined target");
          return;
    }

    switch(RfIntf.ModeTech) { //Indetify card technology
      case (MODE_POLL | TECH_PASSIVE_NFCA):
          Serial.print("\tSENS_RES = ");
          sprintf(tmp, "0x%.2X",RfIntf.Info.NFC_APP.SensRes[0]);
          Serial.print(tmp); Serial.print(" ");
          sprintf(tmp, "0x%.2X",RfIntf.Info.NFC_APP.SensRes[1]);
          Serial.print(tmp); Serial.println(" ");
          
          Serial.print("\tNFCUID = ");
          PrintBuf(RfIntf.Info.NFC_APP.NfcId, RfIntf.Info.NFC_APP.NfcIdLen);
          
          if(RfIntf.Info.NFC_APP.SelResLen != 0) {
              Serial.print("\tSEL_RES = ");
              sprintf(tmp, "0x%.2X",RfIntf.Info.NFC_APP.SelRes[0]);
              Serial.print(tmp); Serial.println(" ");
  
          }
      break;
  
      default:
          break;
    }
    if(RfIntf.MoreTags) { // It will try to identify more NFC cards if they are the same technology
      if(nfc.ReaderActivateNext(&RfIntf) == NFC_ERROR) break;
    }
    else break;
  }
}

void setup(){
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Detect NFC tags with PN7150");
  
  Serial.println("Initializing...");                
  if (nfc.connectNCI()) { //Wake up the board
    Serial.println("Error while setting up the mode, check connections!");
    while (1);
  }
  
  if (nfc.ConfigureSettings()) {
    Serial.println("The Configure Settings is failed!");
    while (1);
  }
  
  if(nfc.ConfigMode(mode)){ //Set up the configuration mode
    Serial.println("The Configure Mode is failed!!");
    while (1);
  }
  nfc.StartDiscovery(mode); //NCI Discovery mode
  Serial.println("Waiting for an Card ...");
}

void loop(){
  if(!nfc.WaitForDiscoveryNotification(&RfInterface)){ // Waiting to detect cards
    displayCardInfo(RfInterface);
    switch(RfInterface.Protocol) {
      case PROT_T2T:
          readntag();
          break;
      
      default:
          break;
    }
    
    //* It can detect multiple cards at the same time if they use the same protocol 
    if(RfInterface.MoreTags) {
        nfc.ReaderActivateNext(&RfInterface);
    }
    //* Wait for card removal 
    nfc.ProcessReaderMode(RfInterface, PRESENCE_CHECK);
    Serial.println("CARD REMOVED!");
    
    nfc.StopDiscovery();
    nfc.StartDiscovery(mode);
  }
  ResetMode();
  delay(500);
}
