/*
  CatSniffer - Use LoRa for communication with the SX1262 module
  
  Eduardo Contreras @ Electronic Cats
  Original Creation Date: Jan 10, 2025

  This code is beerware; if you see me (or any other Electronic Cats
  member) at the local, and you've found our code helpful,
  please buy us a round!
  Distributed as-is; no warranty is given.
*/

#include <RadioLib.h>

#define CTF1 8
#define CTF2 9
#define CTF3 10

#define LED1 (27)
#define LED2 (26)
#define LED3 (28)

uint8_t LEDs[3]={LED1,LED2,LED3};

// SX1262 has the following connections:
// NSS pin:   17
// DIO1 pin:  5
// NRST pin:  24
// BUSY pin:  4
SX1262 radio = new Module(17, 5, 24, 4);

// whether we are receiving, or scanning
bool receiving = false;

// flag to indicate that a packet was detected or CAD timed out
volatile bool scanFlag = false;

void setFlag(void) {
  // something happened, set the flag
  scanFlag = true;
}

void setup() {
  Serial.begin(921600);
  while(!Serial);
  pinMode(CTF1, OUTPUT);
  pinMode(CTF2, OUTPUT);
  pinMode(CTF3, OUTPUT);

  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);

  digitalWrite(CTF1,  HIGH);
  digitalWrite(CTF2,  LOW);
  digitalWrite(CTF3,  LOW);

  digitalWrite(LED1, 0);
  digitalWrite(LED2, 0);
  digitalWrite(LED3, 0);
  // initialize SX1262 with default settings
  Serial.print(F("[SX1262] Initializing ... "));
  int state = radio.begin(915.0, 250, 7, 5, 0x34, 20, 10, 0, false);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
  
  radio.setRfSwitchPins(21, 20);

  // set the function that will be called
  // when LoRa packet or timeout is detected
  radio.setDio1Action(setFlag);

  // start scanning the channel
  Serial.print(F("[SX1262] Starting scan for LoRa preamble ... "));
  state = radio.startChannelScan();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
  
}

void loop() {
  // check if the flag is set
  if(scanFlag) {
    int state = RADIOLIB_ERR_NONE;

    // reset flag
    scanFlag = false;

    // check ongoing reception
    if(receiving) {
      // DIO triggered while reception is ongoing
      // that means we got a packet

      // you can read received data as an Arduino String
      String str;
      state = radio.readData(str);
  
      // you can also read received data as byte array
      /*
        byte byteArr[8];
        state = radio.readData(byteArr, 8);
      */
    
      if (state == RADIOLIB_ERR_NONE) {
        // packet was successfully received
        Serial.println(F("[SX1262] Received packet!"));
  
        // print data of the packet
        Serial.print(F("[SX1262] Data:\t\t"));
        Serial.println(str);
  
        // print RSSI (Received Signal Strength Indicator)
        Serial.print(F("[SX1262] RSSI:\t\t"));
        Serial.print(radio.getRSSI());
        Serial.println(F(" dBm"));
  
        // print SNR (Signal-to-Noise Ratio)
        Serial.print(F("[SX1262] SNR:\t\t"));
        Serial.print(radio.getSNR());
        Serial.println(F(" dB"));
  
        // print frequency error
        Serial.print(F("[SX1262] Frequency error:\t"));
        Serial.print(radio.getFrequencyError());
        Serial.println(F(" Hz"));
  
      } else {
        // some other error occurred
        Serial.print(F("[SX1262] Failed, code "));
        Serial.println(state);
  
      }

      // reception is done now
      receiving = false;
      
    } else {
      // check CAD result
      state = radio.getChannelScanResult();

      if (state == RADIOLIB_LORA_DETECTED) {
        // LoRa packet was detected
        Serial.print(F("[SX1262] Packet detected, starting reception ... "));
        state = radio.startReceive();
        if (state == RADIOLIB_ERR_NONE) {
          Serial.println(F("success!"));
        } else {
          Serial.print(F("failed, code "));
          Serial.println(state);
        }

        // set the flag for ongoing reception
        receiving = true;

      } else if (state == RADIOLIB_CHANNEL_FREE) {
        // channel is free
        Serial.println(F("[SX1262] Channel is free!"));

      } else {
        // some other error occurred
        Serial.print(F("[SX1262] Failed, code "));
        Serial.println(state);

      }

    }

    // if we're not receiving, start scanning again
    if(!receiving) {
      Serial.print(F("[SX1262] Starting scan for LoRa preamble ... "));
      state = radio.startChannelScan();
      if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!"));
      } else {
        Serial.print(F("failed, code "));
        Serial.println(state);
      }
    
    }

  }
    
}
