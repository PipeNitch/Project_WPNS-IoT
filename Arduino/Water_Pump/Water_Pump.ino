#include <PZEM004Tv30.h>
#include <BlynkSimpleEsp8266.h>
//#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WidgetRTC.h>
#include <TimeLib.h>

byte RelayAt = D4;
byte FlowSwitchAt = D5;
unsigned long last;
byte FlowSwitchStatus = 0;
byte isExit = 0;
byte StatusSendLinePump = 0;
byte StatusSendLineNotFlow = 0;
byte StatusSendLineWaterFlow = 0;
float current = 0;

const char* auth = "81PnZkd7Wmmt2rGAmoO7NaqeZ-Jc8kNd";
const char* LToken = "W3bppMPP56vtyf3d1G4WfmKozhzUgwHWfSwE8z6iSlr";
// only 2.4 G
const char* ssid = "Tenda_F1B010";
const char* pass = "kai.kai.kai";

WidgetRTC rtc;
PZEM004Tv30 pzem(D6, D7);
BlynkTimer timer;
//LiquidCrystal_I2C lcd(0x27, 16, 2);

BLYNK_CONNECTED(){rtc.begin();}

unsigned long Delayflow = 0;
unsigned long AmpLimit = 0;
unsigned long DelayAmp = 0;
byte LineNoti = 0;

BLYNK_WRITE(V10) {
  Delayflow=param.asInt();
  EEPROM.write(0,Delayflow);
  EEPROM.commit();
  Delayflow=Delayflow*1000;
  Serial.print("Received Delayflow: ");
  Serial.println(Delayflow);
}

BLYNK_WRITE(V9) {
  AmpLimit=param.asInt();
  EEPROM.write(1,AmpLimit);
  EEPROM.commit();
  Serial.print("Received AmpLimit: ");
  Serial.println(AmpLimit);
}

BLYNK_WRITE(V6) {
  DelayAmp=param.asInt();
  EEPROM.write(2,DelayAmp);
  EEPROM.commit();
  DelayAmp=DelayAmp*1000;
  Serial.print("Received DelayAmp: ");
  Serial.println(DelayAmp);
}

BLYNK_WRITE(V0) {
  LineNoti=param.asInt();
  EEPROM.write(3,LineNoti);
  EEPROM.commit();
  Serial.print("Received LineNoti: ");
  Serial.println(LineNoti);
}



void setup() {
  Serial.begin(115200);
//  WiFi.begin(ssid, pass);
  Blynk.begin(auth, ssid, pass, "blynk.en-26.com", 9600);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting...\n");
    delay(500);
  }
  EEPROM.begin(512);

  timer.setInterval(1000L, clockDisplay);
  
  Delayflow = EEPROM.read(0)*1000;
  AmpLimit = EEPROM.read(1);
  DelayAmp = EEPROM.read(2);
  LineNoti = EEPROM.read(3);
  
  pinMode(FlowSwitchAt, INPUT);
  pinMode(RelayAt, OUTPUT);
//  lcd.init();
//  lcd.display();
//  lcd.backlight();
//  lcd.clear();
//  lcd.print("Pump Status: ");
  last = millis();
  digitalWrite(RelayAt, 1);
}



void clockDisplay(){
  char Time[10];
  sprintf(Time, "%d:%02d:%02d", hour(), minute(), second());
  String Date = String(day()) + "/" + month() + "/" + year();
  Serial.print("Current time: ");
  Serial.print(Time);
  Serial.print(" ");
  Serial.print(Date);
  Serial.println();
  Blynk.virtualWrite(V13, Time);
  Blynk.virtualWrite(V12, Date);
}



void loop() {
  Blynk.run();
  timer.run();
  Blynk.virtualWrite(V4,pzem.current());
  FlowSwitchStatus = digitalRead(D5);
//  Serial.println(pzem.voltage());
//  Serial.println(pzem.current());
  current = pzem.current();
  
  // Check Pump Status
  if(current>1.0){
    // Check FlowSwitch Status
    if(FlowSwitchStatus){
      Serial.println("ON");
      last = millis();
      }
    else {
      Serial.println("OFF");
      if(millis()-last>Delayflow){
        isExit = 1;
      }
    }
  }.
  else{
    last = millis();
  }
  if(isExit){
    digitalWrite(RelayAt, 0);
  }
  delay(100);
}
