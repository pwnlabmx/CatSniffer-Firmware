/*
  CatSniffer - Use LoRa for communication with the SX1262 module
  
  Eduardo Contreras @ Electronic Cats
  Original Creation Date: Jan 10, 2025

  This code is beerware; if you see me (or any other Electronic Cats
  member) at the local, and you've found our code helpful,
  please buy us a round!
  Distributed as-is; no warranty is given.
*/

#define SERIALCOMMAND_HARDWAREONLY
#include <SerialCommand.h> // https://github.com/kroimon/Arduino-SerialCommand
#include <RadioLib.h>

// this file contains binary patch for the SX1262
#include <modules/SX126x/patches/SX126x_patch_scan.h>

#define CTF1 8
#define CTF2 9
#define CTF3 10

#define FREQ_RANGE_START 860
#define FREQ_RANGE_END 928

#define LED1 (27)
#define LED2 (26)
#define LED3 (28)

struct FreqContext{
  float freqStart;
  float freqEnd;
  float freq;
};

uint8_t LEDs[3]={LED1,LED2,LED3};

// SX1262 has the following connections:
// NSS pin:   17
// DIO1 pin:  5
// NRST pin:  24
// BUSY pin:  4
SX1262 radio = new Module(17, 5, 24, 4);

SerialCommand SCmd;

FreqContext freqCtx;

void setup() {
  Serial.begin(921600);
  while(!Serial)

  // Callbacks for serial commands
  SCmd.addCommand("set_freq_start", cmdSetFreqStart);
  SCmd.addCommand("set_freq_end", cmdSetFreqEnd);
  SCmd.setDefaultHandler(unrecognized);  // Handler for command that isn't matched  (says "What?")

  // frequency range in MHz to scan
  freqCtx.freqStart = FREQ_RANGE_START;
  freqCtx.freqEnd = FREQ_RANGE_END;
  
  // initialize SX1262 FSK modem at the initial frequency
  Serial.println(F("DONE: Initializing ... "));
  int state = radio.beginFSK(freqCtx.freqStart,4.8,5.0,156.2,10,16,0,false);
  if(state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("ERROR:"));
    Serial.println(state);
    while (true) { delay(10); }
  }

  // upload a patch to the SX1262 to enable spectral scan
  // NOTE: this patch is uploaded into volatile memory,
  //       and must be re-uploaded on every power up
  Serial.print(F("DONE Uploading patch ... "));
  state = radio.uploadPatch(sx126x_patch_scan, sizeof(sx126x_patch_scan));
  if(state == RADIOLIB_ERR_NONE) {
    Serial.println(F("DONE"));
  } else {
    Serial.print(F("ERROR:"));
    Serial.println(state);
    while (true) { delay(10); }
  }

  // configure scan bandwidth to 234.4 kHz
  // and disable the data shaping
  // Serial.print(F("[SX1262] Setting scan parameters ... "));
  state = radio.setRxBandwidth(234.3);
  state |= radio.setDataShaping(RADIOLIB_SHAPING_NONE);
  if(state == RADIOLIB_ERR_NONE) {
    Serial.println(F("DONE"));
  } else {
    Serial.print(F("ERROR:"));
    Serial.println(state);
    while (true) { delay(10); }
  }
    // some modules have an external RF switch
  // controlled via two pins (RX enable, TX enable)
  // to enable automatic control of the switch,
  // call the following method
  // RX enable:   4
  // TX enable:   5
  
  radio.setRfSwitchPins(21, 20);
  digitalWrite(LED1, 0);
  digitalWrite(LED2, 1);
  digitalWrite(LED3, 0);
}


void cmdSetFreqStart(){
  char *arg;
  arg = SCmd.next();
  float tmpFreq = atoi(arg);
  if (arg != NULL){
    
    if(tmpFreq > freqCtx.freqEnd){
      Serial.println(F("Selected frequency is invalid for this module!"));
      return;
    }
    freqCtx.freqStart = tmpFreq;
    radio.setFrequency(tmpFreq);
  }else{
    Serial.println(F("Invalid argument!"));
  }
}

void cmdSetFreqEnd(){
  char *arg;
  arg = SCmd.next();
  float tmpFreq = atoi(arg);
  if (arg != NULL){
    
    if(tmpFreq < freqCtx.freqStart){
      Serial.println(F("Selected frequency is invalid for this module!"));
      return;
    }
    freqCtx.freqEnd = tmpFreq;
    radio.setFrequency(tmpFreq);
  }else{
    Serial.println(F("Invalid argument!"));
  }
}

void loop() {
  SCmd.readSerial();     // We don't do much, just process serial commands
  // perform scan over the entire frequency range
  freqCtx.freq = freqCtx.freqStart;
  while(freqCtx.freq <= freqCtx.freqEnd) {
    // Serial.print("FREQ ");
    // Serial.println(freq, 2);

    // start spectral scan
    // number of samples: 2048 (fewer samples = better temporal resolution)
    // Serial.print(F("[SX1262] Starting spectral scan ... "));
    int state = radio.spectralScanStart(2048);
    if(state == RADIOLIB_ERR_NONE) {
      Serial.println(F("DONE"));
    } else {
      Serial.print(F("ERROR:"));
      Serial.println(state);
      while (true) { delay(10); }
    }

    // wait for spectral scan to finish
    while(radio.spectralScanGetStatus() != RADIOLIB_ERR_NONE) {
      delay(10);
    }

    // read the results
    uint16_t results[RADIOLIB_SX126X_SPECTRAL_SCAN_RES_SIZE];
    state = radio.spectralScanGetResult(results);
    if(state == RADIOLIB_ERR_NONE) {
      digitalWrite(LED1, 1);
      digitalWrite(LED2, 1);
      digitalWrite(LED3, 1);
      Serial.print("@S");
      Serial.print(freqCtx.freq, 2);
      Serial.print(";");
      for(uint8_t i = 0; i < RADIOLIB_SX126X_SPECTRAL_SCAN_RES_SIZE; i++) {
        Serial.print(results[i]);
        Serial.print(',');
      }
      Serial.println("@E");
    }

    // wait a little bit before the next scan
    delay(5);

    // set the next frequency
    // the frequency step should be slightly smaller
    // or the same as the Rx bandwidth set in setup
    freqCtx.freq += 0.2;
    radio.setFrequency(freqCtx.freq);
    digitalWrite(LED1, 0);
    digitalWrite(LED2, 0);
    digitalWrite(LED3, 0);
  }

  
}

void unrecognized(const char *command) {
  Serial.println("Command not found, type help to get the valid commands");
}
