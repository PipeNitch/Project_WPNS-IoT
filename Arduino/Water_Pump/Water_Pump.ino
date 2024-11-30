#include <PZEM004Tv30.h>
#include <BlynkSimpleEsp8266.h>
#include <TridentTD_LineNotify.h>
//#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WidgetRTC.h>
#include <TimeLib.h>

const char* auth = "81PnZkd7Wmmt2rGAmoO7NaqeZ-Jc8kNd";
const char* LToken = "cZW2GpTn64FenmiTt38AfgMHWvt4XdlLS7gqChQmwnO";

// only 2.4 G
//const char* ssid = "Tenda_F1B010";
//const char* pass = "kai.kai.kai";
const char* ssid = "Inch";
const char* pass = "licht4825!";

WidgetRTC rtc;
PZEM004Tv30 pzem(D6, D7);
BlynkTimer timer;
//LiquidCrystal_I2C lcd(0x27, 16, 2);

BLYNK_CONNECTED(){rtc.begin();}

byte PumpStatus = 0;
byte RelayAt = D4;
byte FlowSwitchAt = D5;
unsigned long last_time;
byte FlowSwitchStatus = 0;
byte isExit = 0;
byte StatusSendLinePump = 0;
byte StatusSendLineNotFlow = 0;
byte StatusSendLineWaterFlow = 0;
float current = 0;

WidgetLED LEDpumpworking(V2);
WidgetLED LEDpumppause(V3);
WidgetLED LEDFlowSwitch(V1);
WidgetLED LEDabnormalcurrent(V5);

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
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting...\n");
    delay(500);
  }
  Blynk.begin(auth, ssid, pass, "blynk.en-26.com", 9600);
  Serial.println(LINE.getVersion());
  
  timer.setInterval(900L, clockDisplay);
  
  EEPROM.begin(512);
  clockDisplay();
  
  Delayflow = EEPROM.read(0)*1000;
  AmpLimit = EEPROM.read(1);
  DelayAmp = EEPROM.read(2)*1000;
  LineNoti = EEPROM.read(3);
  
  LINE.setToken(LToken);
  LINE.notify("Starting System");
  
  pinMode(FlowSwitchAt, INPUT);
  pinMode(RelayAt, OUTPUT);
//  lcd.init();
//  lcd.display();
//  lcd.backlight();
//  lcd.clear();
//  lcd.print("Pump Status: ");
  last_time = millis();
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
      last_time = millis();
      }
    else {
      Serial.println("OFF");
      if(millis()-last_time>Delayflow){
        isExit = 1;
      }
    }
  }
  else{
    last_time = millis();
  }
  if(isExit){
    digitalWrite(RelayAt, 0);
  }
  delay(100);
}
