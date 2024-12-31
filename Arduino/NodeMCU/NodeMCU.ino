#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

char auth[] = "VaeGkPE3Gc_FLfEP0CcJ_dgwMPpV0cd3";
char ssid[] = "M";
char pass[] = "gotodie1";

#define biled LED_BUILTIN

// BLYNK_WRITE(V0)
// {   
//   int value = param.asInt(); // Get value as integer
//   Serial.println(value);
//   if(value==1){
//     digitalWrite(biled,1);
//   }
//   else digitalWrite(biled,0);
// }

LiquidCrystal_I2C lcd(0x27, 16, 2);
void setup(){
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass, "blynk.en-26.com", 9443);

  pinMode(biled, OUTPUT);

  lcd.init();
  lcd.display();
  lcd.backlight();
  lcd.clear();
  
  // if (Blynk.connected()) {
  //   lcd.print("Connected to Blynk server");
  // } else {
  //   lcd.print("Failed to connect to Blynk server");
  // }
}

void loop(){
  Blynk.run();
  // lcd.setCursor(0,1);
  // lcd.print("Test");
}

