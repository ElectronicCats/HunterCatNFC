/**
   Example to detect NFC readers, read a Mifare Card, read a Visa card
   and emulate a Visa MSD card.

   Authors:
          Salvador Mendoza - @Netxing - salmg.net
          For Electronic Cats - electroniccats.com

    January 2021

   This code is beerware; if you see me (or any other collaborator
   member) at the local, and you've found our code helpful,
   please buy us a round!
   Distributed as-is; no warranty is given.
*/
#include "Electroniccats_PN7150.h"

//#define DEBUG

#define PN7150_IRQ   (15)
#define PN7150_VEN   (14)
#define PN7150_ADDR  (0x28)
#define KEY_MFC      (0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF )        // Default Mifare Classic key

Electroniccats_PN7150 nfc(PN7150_IRQ, PN7150_VEN, PN7150_ADDR); // creates a global NFC device interface object, attached to pins 7 (IRQ) and 8 (VEN) and using the default I2C address 0x28
RfIntf_t RfInterface;
uint8_t mode = 2;                                                  // modes: 1 = Reader/ Writer, 2 = Emulation

unsigned char STATUSOK[] = {0x90, 0x00}, Cmd[256], CmdSize;

// Token = data to be use it as track 2
// 4412345605781234 = card number in this case
uint8_t token[19] = {0x44, 0x12, 0x34, 0x56, 0x05 , 0x78, 0x12, 0x34, 0xd1, 0x71, 0x12, 0x01, 0x00, 0x00, 0x03, 0x00, 0x00, 0x99, 0x1f};

//Visa MSD emulation variables
uint8_t apdubuffer[255] = {}, apdulen;
uint8_t ppsea[] = {0x6F, 0x23, 0x84, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0xA5, 0x11, 0xBF, 0x0C, 0x0E, 0x61, 0x0C, 0x4F, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0x87, 0x01, 0x01, 0x90, 0x00};
uint8_t visaa[] = {0x6F, 0x1E, 0x84, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0xA5, 0x13, 0x50, 0x0B, 0x56, 0x49, 0x53, 0x41, 0x20, 0x43, 0x52, 0x45, 0x44, 0x49, 0x54, 0x9F, 0x38, 0x03, 0x9F, 0x66, 0x02, 0x90, 0x00};
uint8_t processinga[] = {0x80, 0x06, 0x00, 0x80, 0x08, 0x01, 0x01, 0x00, 0x90, 0x00};
uint8_t last [4] =  {0x70, 0x15, 0x57, 0x13};
uint8_t card[25] = {};
uint8_t statusapdu[2] = {0x90, 0x00};
uint8_t finished[] = {0x6f, 0x00};

#define BLK_NB_MFC      4                                          // Block tat wants to be read
#define KEY_MFC         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF         // Default Mifare Classic key

boolean detectCardFlag = false;

uint8_t ppdol[255] = {0x80, 0xA8, 0x00, 0x00, 0x02, 0x83, 0x00};

int resetMode() { //Reset the configuration mode after each reading
  Serial.println("Reset...");
  if (nfc.connectNCI()) { //Wake up the board
    Serial.println("Error while setting up the mode, check connections!");
    while (1);
  }

  if (nfc.ConfigureSettings()) {
    Serial.println("The Configure Settings failed!");
    while (1);
  }

  if (nfc.ConfigMode(mode)) { //Set up the configuration mode
    Serial.println("The Configure Mode failed!!");
    while (1);
  }

  nfc.StartDiscovery(mode); //NCI Discovery mode
}

void blink(int pin, int msdelay, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(msdelay);
    digitalWrite(pin, LOW);
    delay(msdelay);
  }
}

//Print hex data buffer in format
void printBuf(const byte * data, const uint32_t numBytes) {
  uint32_t szPos;
  for (szPos = 0; szPos < numBytes; szPos++) {
    Serial.print(F("0x"));
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      Serial.print(F("0"));

    Serial.print(data[szPos] & 0xff, HEX);
    if ((numBytes > 1) && (szPos != numBytes - 1)) {
      Serial.print(F(" "));
    }
  }
  Serial.println();
}
/*
    treatPDOL function:
   Make a right format challenge using the card PDOL to extract more data(track 2)
   Note: This challenge only follows the format, do not use it as real challenge generator
*/
uint8_t treatPDOL(uint8_t* apdu) {
  uint8_t plen = 7;
  Serial.println("");
  //PDOL Format: 80 A8 00 00 + (Tamaño del PDOL+2) + 83 + Tamaño del PDOL + PDOL + 00
  for (uint8_t i = 1; i <= apdu[0]; i++) {
    if (apdu[i] == 0x9F && apdu[i + 1] == 0x66) {
      ppdol[plen] = 0xF6;
      ppdol[plen + 1] = 0x20;
      ppdol[plen + 2] = 0xC0;
      ppdol[plen + 3] = 0x00;
      plen += 4;
      i += 2;
    }
    else if (apdu[i] == 0x9F && apdu[i + 1] == 0x1A) {
      ppdol[plen] = 0x9F;
      ppdol[plen + 1] = 0x1A;
      plen += 2;
      i += 2;
    }
    else if (apdu[i] == 0x5F && apdu[i + 1] == 0x2A) {
      ppdol[plen] = 0x5F;
      ppdol[plen + 1] = 0x2A;
      plen += 2;
      i += 2;
    }
    else if (apdu[i] == 0x9A) {
      ppdol[plen] = 0x9A;
      ppdol[plen + 1] = 0x9A;
      ppdol[plen + 2] = 0x9A;
      plen += 3;
      i += 1;
    }
    else if (apdu[i] == 0x95) {
      ppdol[plen] = 0x95;
      ppdol[plen + 1] = 0x95;
      ppdol[plen + 2] = 0x95;
      ppdol[plen + 3] = 0x95;
      ppdol[plen + 4] = 0x95;
      plen += 5;
      i += 1;
    }
    else if (apdu[i] == 0x9C) {
      ppdol[plen] = 0x9C;
      plen += 1;
      i += 1;
    }
    else if (apdu[i] == 0x9F && apdu[i + 1] == 0x37) {
      ppdol[plen] = 0x9F;
      ppdol[plen + 1] = 0x37;
      ppdol[plen + 2] = 0x9F;
      ppdol[plen + 3] = 0x37;
      plen += 4;
      i += 2;
    }
    else {
      uint8_t u = apdu[i + 2];
      while (u > 0) {
        ppdol[plen] = 0;
        plen += 1;
        u--;
      }
      i += 2;
    }
  }
  ppdol[4] = (plen + 2) - 7; // Length of PDOL + 2
  ppdol[6] = plen - 7;       // Real length
  plen++;                    // +1 because the last 0
  ppdol[plen] = 0x00;        // Add the last 0 to the challenge
  return plen;
}

void printData(uint8_t* buff, uint8_t lenbuffer, uint8_t cmd) {
  char tmp[1];
  if (cmd == 1)
    Serial.print("\nCommand: ");
  else if (cmd == 2)
    Serial.print("\nReader command: ");
  else if (cmd == 3)
    Serial.print("\nHunter Cat answer: ");
  else
    Serial.print("\nCard answer: ");

  for (uint8_t u = 0; u < lenbuffer; u++) {
    sprintf(tmp, "0x%.2X", buff[u]);
    Serial.print(tmp); Serial.print(" ");
  }
}

//Find Track 2 in the NFC reading transaction
void seekTrack2() {
  bool exc, chktoken = false, existpdol = false;
  uint8_t apdubuffer[255] = {}, apdulen;

  uint8_t ppse[] = {0x00, 0xA4, 0x04, 0x00, 0x0e, 0x32, 0x50, 0x41, 0x59, 0x2e, 0x53, 0x59, 0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00}; //20
  uint8_t visa[] = {0x00, 0xA4, 0x04, 0x00, 0x07, 0xa0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0x00}; //13
  uint8_t processing [] = {0x80, 0xA8, 0x00, 0x00, 0x02, 0x83, 0x00, 0x00}; //8
  uint8_t sfi[] = {0x00, 0xb2, 0x01, 0x0c, 0x00}; //5

  uint8_t *apdus[] = {ppse, visa, processing, sfi};
  uint8_t apdusLen [] = { sizeof(ppse), sizeof(visa), sizeof(processing), sizeof(sfi)};

  uint8_t pdol[50], plen = 8;

  for (uint8_t i = 0; i < 4; i++) {
    //blink(L2, 150, 1);
    nfc.CardModeSend(apdus[i], apdusLen[i]);

    while (nfc.CardModeReceive(apdubuffer, &apdulen) != 0) { }

    if (nfc.CardModeReceive(apdubuffer, &apdulen) == 0) {
      printData(apdus[i], apdusLen[i], 1);
      printData(apdubuffer, apdulen, 4);
      for (uint8_t u = 0; u < apdulen; u++) {
        if (i == 1) {
          if (apdubuffer[u] == 0x9F && apdubuffer[u + 1] == 0x38) {
            for (uint8_t e = 0; e <= apdubuffer[u + 2]; e++)
              pdol[e] =  apdubuffer[u + e + 2];

            plen = treatPDOL(pdol);
            apdus[2] = ppdol;
            apdusLen[2] = plen;
            existpdol = true;
          }
        }
        else if (i == 3) {
          if (apdubuffer[u] == 0x57 && apdubuffer[u + 1] == 0x13 && !chktoken) {
            chktoken = true;
            memcpy(&token, &apdubuffer[u + 2], 19);
            break;
          }
        }
      }
      if (i == 1) {
        char tmp[1];
        Serial.print("\nFull challenge: ");
        for (uint8_t b = 0; b < plen; b++) {
          sprintf(tmp, "0x%.2X", existpdol ? ppdol[b] : processing[b]);
          Serial.print(tmp); Serial.print(" ");
        }
        Serial.println("");
      }
      Serial.println("");
    }
    else
      Serial.println("Error reading the card!");

  }
}

// Read Mifare Classic 1K
void readingmifare(void) {
  uint8_t success = 1;                          // Flag to check if there was an error
  uint8_t currentblock = 0;                     // Counter to keep track of which block we're on
  bool authenticated = false;                   // Flag to indicate if the sector is authenticated
  unsigned char respm[16];
  unsigned char respsize;
  unsigned char authm[] = {0x40, currentblock / 4, 0x10, KEY_MFC};

  /* Read block 4 */
  unsigned char readm[] = {0x10, 0x30, currentblock};
  Serial.println("Reading Mifare...");
  // Now we try to go through all 16 sectors (each having 4 blocks)
  // authenticating each sector, and then dumping the blocks
  for (currentblock = 0; currentblock < 64; currentblock++) {
    // Check if this is a new block so that we can reauthenticate

    /* Authenticate sector 1 with generic keys */
    //authenticated = true;
    authm[1] = currentblock / 4;
    authenticated = nfc.ReaderTagCmd(authm, sizeof(authm), respm, &respsize);

    if ((authenticated == NFC_ERROR) || (respm[respsize - 1] != 0)) {
      authenticated = false;
    } else {
      authenticated = true;
    }
    // If we're still not authenticated just skip the block
    if (!authenticated) {
      Serial.print("Block "); Serial.print(currentblock, DEC);
      Serial.println(" unable to authenticate");
    } else {
      // Authenticated ... we should be able to read the block now
      // Dump the data into the 'data' array
      readm[2] = {currentblock};
      success = nfc.ReaderTagCmd(readm, sizeof(readm), respm, &respsize);
      if ((success == NFC_ERROR) || (respm[respsize - 1] != 0)) {
        success = 0;
      } else {
        success = 1;
      }
      if (success) {
        // Read successful
        Serial.print("Block "); Serial.print(currentblock, DEC);
        if (currentblock < 10)
          Serial.print("  ");
        else
          Serial.print(" ");

        // Dump the raw data
        //nfc.PrintHexChar(data, 16);

        printBuf(respm + 1, respsize - 2);
      } else {
        // Oops ... something happened
        Serial.print("Block ");
        Serial.print(currentblock, DEC);
        Serial.println(" unable to read this block");
      }
    }
    delay(50);
  }
  Serial.println("Finish Dump Card...");
}

//Is it a card in range? for Mifare and ISO cards
void detectcard() {
  while (detectCardFlag == false) {
    Serial.println("wait detect Card...");
    if (!nfc.WaitForDiscoveryNotification(&RfInterface)) { // Waiting to detect cards

      if (RfInterface.ModeTech == MODE_POLL || RfInterface.ModeTech == TECH_PASSIVE_NFCA) {
        char tmp[16];

        Serial.print("\tSENS_RES = ");
        sprintf(tmp, "0x%.2X", RfInterface.Info.NFC_APP.SensRes[0]);
        Serial.print(tmp); Serial.print(" ");
        sprintf(tmp, "0x%.2X", RfInterface.Info.NFC_APP.SensRes[1]);
        Serial.print(tmp); Serial.println(" ");
        Serial.print("\tNFCID = ");
        printBuf(RfInterface.Info.NFC_APP.NfcId, RfInterface.Info.NFC_APP.NfcIdLen);

        if (RfInterface.Info.NFC_APP.NfcIdLen != 4) {

          Serial.println("Ooops ... this doesn't seem to be a Mifare Classic card!");
          return;
        }

        if (RfInterface.Info.NFC_APP.SelResLen != 0) {

          Serial.print("\tSEL_RES = ");
          sprintf(tmp, "0x%.2X", RfInterface.Info.NFC_APP.SelRes[0]);
          Serial.print(tmp); Serial.println(" ");
        }
      }
      switch (RfInterface.Protocol) {
        case PROT_ISODEP:

          Serial.println(" - Found ISODEP card");

          seekTrack2();
          break;

        case PROT_MIFARE:
          Serial.println(" - Found MIFARE card");

          readingmifare();
          break;

        default:
          Serial.println(" - Not a valid card");
          break;
      }

      //* It can detect multiple cards at the same time if they use the same protocol
      if (RfInterface.MoreTags) {
        nfc.ReaderActivateNext(&RfInterface);
      }

      //* Wait for card removal
      nfc.ProcessReaderMode(RfInterface, PRESENCE_CHECK);
      Serial.println("CARD REMOVED!");

      nfc.StopDiscovery();
      nfc.StartDiscovery(mode);
      detectCardFlag = true;
    }
  }
}

//Detect NFC Readers
void nfcdetectreader() {
  mode = 2;
  resetMode();
  while(detectCardFlag==false){
  Serial.println("Looking for card readers...");
  if (nfc.CardModeReceive(Cmd, &CmdSize) == 0) { //Data in buffer?
    if ((CmdSize >= 2) && (Cmd[0] == 0x00)) { //Expect at least two bytes
      switch (Cmd[1]) {
        case 0xA4: //Something tries to select a file, meaning that it is a reader
          Serial.println("Card reader detected!");
          detectCardFlag=true;
          break;

        default:
          break;
      }
      nfc.CardModeSend(STATUSOK, sizeof(STATUSOK));
    }
  }
  }
}

//To read Mifare and Visa
void mifarevisa() {
  mode = 1;
  resetMode();
  detectcard();
  detectCardFlag = false;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Detecting NFC readers with PN7150");

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);

  pinMode(BUTTON_0, INPUT_PULLUP);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PIN_LED2, LOW);
  digitalWrite(PIN_LED2, LOW);

  resetMode();
  Serial.println("HunterCat NFC v1.0");
}

// to detect card readers: nfcdetectreader()
// to read visa card: mifarevisa()
// to emulate Visa MSD: visamsd()
// to read Mifare card: mifarevisa()

void loop() {
  if (digitalRead(BUTTON_0) == 0) {
    // to detect card readers: nfcdetectreader()
    Serial.println("nfcdetectreader");
    nfcdetectreader();
  }
  if (digitalRead(BUTTON_1) == 0) {
    // to read visa card: mifarevisa()
    Serial.println("mifarevisa");
    mifarevisa();
    delay(100);
  }
  if (digitalRead(BUTTON_2) == 0) {
    // to emulate Visa MSD: visamsd()
    Serial.println("visamsd");
    //visamsd();
  }
}
