# Hunter Cat NFC

<a href="https://electroniccats.com/store/hunter-cat-nfc/">
  <p align="center">
  <img src="https://electroniccats.com/wp-content/uploads/badge_store.png" height="104"  />
  </p>
</a>

## How does the Hunter Cat NFC work?

The Hunter Cat NFC is a security tool for contactless (Near Field Communication) used in access control, identification, and bank cards. It is specially created to identify NFC readers and sniffing tools. With this tool, you can audit, read or emulate cards of different types.

## Understanding the Hunter Cat NFC and its LEDs 

The device has preloaded reader detection firmware that lets the user know when is near an NFC reader attempting to read his card.

### Reader detection 

To detect hidden readers seeking to read your cards without authorization for cloning.

1. Turn on Hunter Cat NFC
2. Wait for the LEDs to turn off
3. Red LED flashes every 1 second
4. Approach the reader, the Hunter Cat NFC LEDs should light up indicating if an NFC reader was detected
5. If it does not turn on and the LED stays on, it is not near an NFC reader

## How does it work? 

Hunter Cat NFC can be set to behave either as an NFC reader, a tag, or to establish a two-way connection with another NFC device.

NFC USB Dongle features a SAMD21 MCU which works in conjunction with the PN7150. The USB interface is provided by SAMD21 MCU, and the NFC functionality is ensured thanks to PN7150.

NFC is designed to be intuitive for users. The communication between two devices is established in the simplest way possible — by bringing them close to each other. NFC frontend can operate in three distinct modes:

### Card emulation mode — where Hunter Cat NFC behaves like a smart card or a tag

In this mode, Hunter Cat NFC emulates a NFC tag. It doesn’t initiate the communication, it only responds to a NFC reader. A typical application of the card emulation mode is how people use NFC in their smartphones to replace several cards, badges or tags at once (using the same phone for RFID access controls, contactless payments and so on). Card emulation mode, however, is not limited to smartphones but can be used for any portable device.

### Read/Write — where Hunter Cat NFC behaves as an NFC Reader/Writer

Here, Hunter Cat NFC communicates with a passive tag, NFC smart card, or an NFC device operating in card emulation mode. It can read or write to a tag (although reading is a more common use-case because tags will often be write protected). In this mode, the Hunter Cat NFC generates the RF field, while a tag or card only modulates it.

### Peer-to-peer — for more complex interactions

Peer-to-peer mode establishes a two-way communication channel between a pair of NFC-enabled devices. When a user brings two devices close to each other, the two NFC chips establish a P2P connection and exchange data.

## Characteristics:
- Procesador Cortex M0+
- USB C 2.0
- NFC Reader, Card and NFC Forum
- Arduino compatible
- CircuitPython compatible
- UF2 Bootloader
- 3 LEDs status
- Open Hardware
- Battery
- RF protocols supported
- NFCIP-1, NFCIP-2 protocol 
- ISO/IEC 14443A, ISO/IEC 14443B PICC, NFC Forum T4T modes via host interface
- NFC Forum T3T via host interface
- ISO/IEC 14443A, ISO/IEC 14443B PCD designed according to NFC Forum digital protocol T4T platform and ISO-DEP 
- FeliCa PCD mode
- MIFARE Classic PCD encryption mechanism (MIFARE Classic 1K/4K)
- NFC Forum tag 1 to 5 (MIFARE Ultralight, Jewel, Open FeliCa tag, MIFAREDESFire
- ISO/IEC 15693/ICODE VCD mode 
- Includes NXP ISO/IEC14443-A and Innovatron ISO/IEC14443-B intellectual property licensing rights

##  Wiki and Getting Started
[**Getting Started in our Wiki**](https://github.com/ElectronicCats/HunterCatNFC/wiki)

<a href="https://github.com/ElectronicCats/HunterCatNFC/wiki">
  <img src="https://user-images.githubusercontent.com/107638696/216455658-128e4589-53a3-4a92-9ddd-d264ae84368a.jpg" height="400" />
</a>

## Disclaimer
Hunter Cat NFC, is a wireless penetration testing tool intended solely for use in authorized security audits, where such usage is permitted by applicable laws and regulations. Before utilizing this tool, it is crucial to ensure compliance with all relevant legal requirements and obtain appropriate permissions from the relevant authorities.

It is essential to note that the board does not provide any means or authorization to utilize credit cards or engage in any financial transactions that are not legally authorized. Electronic Cats holds no responsibility for any unauthorized use of the tool or any resulting damages.

## License
<a>
  <img src="https://github.com/ElectronicCats/AjoloteBoard/raw/master/OpenSourceLicense.png" height="150" />
</a>
Electronic Cats invests time and resources in providing this open-source design. Please support Electronic Cats and open-source hardware by purchasing products from Electronic Cats!

Designed by Electronic Cats.

Firmware released under an GNU AGPL v3.0 license. See the LICENSE file for more information.

Hardware released under an CERN Open Hardware Licence v1.2. See the LICENSE_HARDWARE file for more information.

Electronic Cats is a registered trademark. Please do not use if you sell these PCBs.

November 2019
