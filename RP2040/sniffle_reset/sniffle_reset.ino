#include "SerialPassthroughwithboot.h"
unsigned long baud = 2000000;

  byte sync_command[] = {'@', '@', '@', '@','@','@','@','@','\r', '\n'};
  byte reset_command[] = {'A', 'R', 'c', '=', '\r', '\n'};
  byte reset2_command[] = {'A', 'h', 'h', 'x', 'Z', 'e','N','v','\r','\n'};
  byte firmware_v_command1[] = {'A', 'R', 'h', 'A', '\r', '\n'};
  byte firmware_v_command2[] = {'A', 'S', 'Q', '=', '\r', '\n'};

void setup() {
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

  Serial.begin(baud);
  Serial1.begin(baud);




  // Outer loop to repeat the command 5 times
  for (int j = 0; j < 5; j++) {
    // Inner loop to send each byte of the command
    for (int i = 0; i < sizeof(reset_command); i++) {
      Serial.write(reset_command[i]);
    }
  }

    for (int i = 0; i < sizeof(sync_command); i++) {
    Serial1.write(firmware_v_command1[i]);
  }
      for (int i = 0; i < sizeof(sync_command); i++) {
    Serial1.write(firmware_v_command1[i]);
  }

    for (int i = 0; i < sizeof(firmware_v_command1); i++) {
    Serial1.write(firmware_v_command1[i]);
  }

  listenForSerial1(500); 



    for (int i = 0; i < sizeof(firmware_v_command2); i++) {
    Serial1.write(firmware_v_command1[i]);
  }

 listenForSerial1(500); 
  
}



void loop() {
  

  /*
    for (int i = 0; i < sizeof(firmware_v_command1); i++) {
    Serial.write(firmware_v_command1[i]);
  }

  for (int i = 0; i < sizeof(firmware_v_command2); i++) {
    Serial.write(firmware_v_command2[i]);
  }
  
  
  if (Serial1.available()) {     // If anything comes in Serial1 (pins 0 & 1)
    Serial.write(Serial1.read());   // read it and send it out Serial (USB)
    delay(10);
  }

  */
  

}

void listenForSerial1(unsigned long duration) {
  unsigned long startTime = millis();
  while (millis() - startTime < duration) {
    if (Serial1.available()) {
      int incomingByte = Serial1.read();
      Serial.write(incomingByte);  // Send it out to Serial (USB)
    }
  }
}

/*
C:\Users\suppo\OneDrive\Documentos\GitHub\Sniffle\python_cli>python reset.py -s COM90
b'ARhA\r\n'
Sending reset commands...
b'ARc=\r\n'
b'ARc=\r\n'
b'ARc=\r\n'
b'ARc=\r\n'
b'ARc=\r\n'
Trying a mark and flush to get things flowing...
b'AhhxZeNv\r\n'
Reset success.
*/

/*
C:\Users\suppo\OneDrive\Documentos\GitHub\Sniffle\python_cli>python3 version_check.py -s COM90
b'ARhA\r\n'
b'ASQ=\r\n'
Timeout probing firmware version
*/