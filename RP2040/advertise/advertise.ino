#include "Base64.h"
#include "SerialPassthroughwithboot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

//#define TEST;

catsniffer_t catsniffer;

uint8_t LEDs[3]={LED1,LED2,LED3};
int i=0;

// Define advertisement data, hardcoded for now

uint8_t advData[] = {
  0x02, 0x01, 0x1A, 0x02, 0x0A, 0x0C, 0x11, 0x07,
  0x64, 0x14, 0xEA, 0xD7, 0x2F, 0xDB, 0xA3, 0xB0,
  0x59, 0x48, 0x16, 0xD4, 0x30, 0x82, 0xCB, 0x27,
  0x05, 0x03, 0x0A, 0x18, 0x0D, 0x18
};

/*
uint8_t advData[] = {
  0x1E, 0xFF, 0x4C, 0x00, 0x12, 0x19, 0x10, 0x62,
  0x99, 0x96, 0x85, 0xAB, 0x4C, 0x0A, 0xB4, 0xA1,
  0xD6, 0xA4, 0x13, 0xA1, 0x9B, 0x30, 0x84, 0x4D,
  0x60, 0x70, 0x59, 0xCF, 0xA2, 0x01, 0x45
};
*/

//Define the scan response data, hardcoded for now 
String devName = "CatSniffer"; //0b 09 43 61 74 53 6e 69 66 66 65 72 Define the name of the device
//uint8_t scanRspData[] = {0x0B, 0x09, 0x43, 0x61, 0x74, 0x53, 0x6E, 0x69, 0x66, 0x66, 0x65, 0x72}; // Name hard coded, lenght of name + 0x09 + CatSniffer in Hex


//Function definitions
int8_t cmdAdvertise(uint8_t * advData, uint8_t advDataLen, uint8_t * scanRspData, uint8_t scanRspDataLen, uint8_t mode);

void cmdSend(int mode, uint8_t* paddedAdvData, uint8_t* paddedScanRspData);

void setup(){   

  uint8_t advDataLen = sizeof(advData) / sizeof(advData[0]);

  uint8_t devNameLen = devName.length() + 1;
  uint8_t scanRspData[devNameLen +2]; 
  scanRspData[0] = devNameLen;
  scanRspData[1] = 0x09;
  for (int i = 0; i < devNameLen; i++) { //Copy each value of advData to paddedAdvData starting from second value
    scanRspData[i+2] = devName[i];
  }

  uint8_t scanRspDataLen = sizeof(scanRspData) / sizeof(scanRspData[0]);


    catsniffer.led_interval=1000;
    catsniffer.baud=921600;
    catsniffer.mode=PASSTRHOUGH;  
    
    #ifdef TEST  
      Serial.begin(catsniffer.baud);
    #endif
    Serial1.begin(catsniffer.baud);

    while (!Serial) ;
    
    pinMode(Pin_Button, INPUT_PULLUP);
    pinMode(Pin_Boot, INPUT_PULLUP);
    pinMode(Pin_Reset, OUTPUT);
    pinMode(Pin_Reset_Viewer, INPUT);
    digitalWrite(Pin_Reset, HIGH);
      
    pinMode(LED1,OUTPUT);
    pinMode(LED2,OUTPUT);
    pinMode(LED3,OUTPUT);
    pinMode(CTF1, OUTPUT);
    pinMode(CTF2, OUTPUT);
    pinMode(CTF3, OUTPUT);
    

    //Make all cJTAG pins an input 
    for(int i=11;i<15;i++){
      pinMode(i,INPUT);
    }
    
    
    changeBand(&catsniffer, GIG);
    
#ifdef TEST
    Serial.println("Advertisement Data is:");
    for(int i = 0; i < advDataLen; i++)
    {
      if(advData[i]>0x0F)
        Serial.print("0x");
      else
        Serial.print("0x0");
      Serial.print(advData[i],HEX);
      Serial.print(" ");
    }
    Serial.print("\n");
    
    Serial.println("Scan Response Data is:");
    for(int i = 0; i < scanRspDataLen; i++)
    {
      if(scanRspData[i]>0x0F)
        Serial.print("0x");
      else
        Serial.print("0x0");
      Serial.print(scanRspData[i],HEX); //Add , HEX again if needed
      Serial.print(" ");
    }
      Serial.print("\n");
#endif
    
    int8_t error = cmdAdvertise(advData, advDataLen, scanRspData, scanRspDataLen, 0); /////////////////////////////////////////////////////////////////// MODE ////////////////////////////////////////////////////////////////////////

  #ifdef TEST
    //Error handling
    if (error == -1)
    {
      Serial.println("Error: advData too long");
    }  
    else if (error == -2)
    {
      Serial.println("Error: scanRspData too long");
    }  
    else if (error == -3)
    {
      Serial.println("Error: Mode must be 0 (connectable), 2 (non-connectable), or 3 (scannable)");
    } 
  #endif
  
    //Listen for response from the cc1352 
    listenForSerial1(1500);
    
}

void loop() {
  

    if(millis() - catsniffer.previousMillis > catsniffer.led_interval) {
    catsniffer.previousMillis = millis(); 
    if(catsniffer.mode){
      digitalWrite(LEDs[i], !digitalRead(LEDs[i]));
      i++;
      if(i>2)i=0;
    }else{
      digitalWrite(LED3, !digitalRead(LED3));
    }
  }

}



int8_t cmdAdvertise(uint8_t * advData, uint8_t advDataLen, uint8_t * scanRspData, uint8_t scanRspDataLen, uint8_t mode) {
    if (advDataLen > 31) {
        return -1;// Error: advData too long
    }

    if (scanRspDataLen > 31) {
        return -2;// Error: scanRspData too long
    }

    if (mode != 0 && mode != 2 && mode != 3) {
        return -3;// Error: Mode must be 0 (connectable), 2 (non-connectable), or 3 (scannable)
    }
  
    
#ifdef TEST
    Serial.print("advDataLen: ");
    Serial.println(advDataLen);

    Serial.print("scanRspDataLen: ");
    Serial.println(scanRspDataLen);
#endif

    uint8_t paddedAdvData[32]={0}; //Creat Padded Advertisement Data Array
    uint8_t paddedScanRspData[32]={0}; //Creat Padded Scan Response Data Array

    //Pass data from one array to another

    //make first value of paddedAdvData advDataLen
    paddedAdvData[0] = advDataLen; 
    //Copy each value of advData to paddedAdvData starting from second value
    for (int i = 0; i < advDataLen; i++) {
      paddedAdvData[i +1] = advData[i];
    }
    //Fill with 0s
    for (int i = advDataLen +1; i < 32; i++) { 
      paddedAdvData[i] = 0;
    }

    //make first value of paddedScanRspData 
    paddedScanRspData[0] = scanRspDataLen; 
    //Copy each value of scanRspData to paddedScanRspData starting from second value
    for (int i = 0; i < scanRspDataLen; i++) { 
      paddedScanRspData[i + 1] = scanRspData[i];
    }
    //Fill with 0s
    for (int i = scanRspDataLen +1; i < 32; i++) { 
      paddedScanRspData[i] = 0;
    }

#ifdef TEST
    Serial.println("Padded Advertisement Data is:");
    for(int i = 0; i < 32; i++)
    {
      if(paddedAdvData[i]>0x0F)
        Serial.print("0x");
      else
        Serial.print("0x0");
      Serial.print(paddedAdvData[i],HEX);
      Serial.print(" ");

    }
      Serial.print("\n");

    Serial.println("Padded Scan Response Data is:");
    for(int i = 0; i < 32; i++)
    {
      if(paddedScanRspData[i]>0x0F)
        Serial.print("0x");
      else
        Serial.print("0x0");
      Serial.print(paddedScanRspData[i],HEX);
      Serial.print(" ");
    }
    Serial.print("\n");
#endif


    cmdSend(mode,paddedAdvData,paddedScanRspData);

    return 0;
}


void cmdSend(int mode, uint8_t * paddedAdvData, uint8_t * paddedScanRspData) {
    // Create a byte array to hold the command
    uint8_t cmdByteList[66]; //[1 + 1 + 32 + 32]
    
    // Assign b0 to the first byte of cmd
    cmdByteList[0] = 0X1C;
    
    // Copy mode byte to cmd
    cmdByteList[1] = mode;
    
    // Copy paddedAdvData into cmd starting from index 2
    for (int i = 2; i < 34; i++) {
        cmdByteList[i] = paddedAdvData[i-2];
    }
    
    // Copy paddedScanRspData into cmd starting from index 2 + advDataLength
    for (int i = 34; i < 66; i++) {
        cmdByteList[i] = paddedScanRspData[i-34];
    }
    // For Testing
#ifdef TEST
    Serial.println("Command Byte List Data is:");
    for(int i = 0; i < 66; i++)
    {
      if(cmdByteList[i]>0x0F)
        Serial.print("0x");
      else
        Serial.print("0x0");
      Serial.print(cmdByteList[i],HEX);
      Serial.print(" ");
    }
      Serial.print("\n");
#endif
    //
    uint8_t cmdByteListLen = sizeof(cmdByteList) / sizeof(cmdByteList[0]);

    int b0=(cmdByteListLen+3)/3; //Create the valuo for cmd[0]'b' which is the lenght of the command 

#ifdef TEST
    Serial.println("b0: ");
    Serial.println(b0);
#endif

    char cmd[cmdByteListLen+1]; //Create the command array with lenght of the byte list + 1 for b0

    cmd[0]=b0; //Make b0 the value for cmd[0]

    for (int i = 1; i < 67; i++) { //Copy the rest of the command data as 
    cmd[i] = cmdByteList[i-1];
    }

    int cmdLen =  sizeof(cmd) / sizeof(cmd[0]); //Get the cmd length

#ifdef TEST
    //For testing only
    Serial.println("Command Data is:");
    for(int i = 0; i < cmdLen; i++)
    {
      if(cmd[i]>0x0F)
        Serial.print("0x");
      else
        Serial.print("0x0");
      Serial.print(cmd[i],HEX);
      Serial.print(" ");
    }
      Serial.print("\n");
#endif

    //Create a variable to hold the mesage and calculate the length of that message using the library function
    int msgLen = base64_enc_len(cmdLen);
    char msg[msgLen];

    base64_encode(msg, cmd, cmdLen); 


    //For testing only
#ifdef TEST
    Serial.println("MSG Data is:");
    for(int i = 0; i < msgLen; i++)
    {
      //if(msg[i]>0x0F)
        //Serial.print("0x");
      //else
        //Serial.print("0x0");
      Serial.print(msg[i],HEX);
      Serial.print(" ");
    }
    Serial.print("\n");
#endif
    
     
    // Write the encoded message to serial
    Serial1.write(msg, msgLen);
    Serial1.write("\r\n"); //send the carriage return and line 

       
}


void changeBand(catsniffer_t *cs, unsigned long newBand){
  if(newBand==cs->band)
    return;
  switch(newBand){
    case GIG:   //2.4Ghz CC1352
      digitalWrite(CTF1,  LOW);
      digitalWrite(CTF2,  HIGH);
      digitalWrite(CTF3,  LOW);
    break;

    case SUBGIG_1: //Sub-ghz CC1352
      digitalWrite(CTF1,  LOW);
      digitalWrite(CTF2,  LOW);
      digitalWrite(CTF3,  HIGH);
    break;

    case SUBGIG_2: //LoRa
      digitalWrite(CTF1,  HIGH);
      digitalWrite(CTF2,  LOW);
      digitalWrite(CTF3,  LOW);
    break;

    default:
    break;
    }

  return;
  }

void listenForSerial1(unsigned long duration) {
unsigned long startTime = millis();
while (millis() - startTime < duration) {
  if (Serial1.available()) {
    int incomingByte = Serial1.read(); //read serial and save it to a variable
   }
  }
}
