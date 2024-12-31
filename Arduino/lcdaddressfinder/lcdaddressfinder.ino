<<<<<<< HEAD
//====== I2C Addresss Scan
#include <Wire.h>

void setup() {
  Serial.begin (115200);
  Serial.println ("start...");
  while (!Serial) {}

  Serial.println ("I2C scanner. Scanning ...");
  byte count = 0;

  Wire.begin();

  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0) {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      delay (1);
    }
  }

  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");
}

void loop() {

=======
//====== I2C Addresss Scan
#include <Wire.h>

void setup() {
  Serial.begin (115200);
  Serial.println ("start...");
  while (!Serial) {}

  Serial.println ("I2C scanner. Scanning ...");
  byte count = 0;

  Wire.begin();

  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0) {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      delay (1);
    }
  }

  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");
}

void loop() {

>>>>>>> c9b41f7 (12/21/2024)
}