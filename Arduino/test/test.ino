#include <PZEM004Tv30.h>
#include <BlynkSimpleEsp8266.h>
#include <TridentTD_LineNotify.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WidgetRTC.h>
#include <TimeLib.h>
#include <Keypad_I2C.h>

byte FlowSwitchAt = D5;
byte FlowSwitchStatus = 0;
BlynkTimer TimerSensor;

void setup() {
  Serial.begin(115200);
  pinMode(FlowSwitchAt, INPUT);
  TimerSensor.setInterval(1000L, ReadSensor);
}

void ReadSensor() {
  FlowSwitchStatus = digitalRead(FlowSwitchAt);
}


void loop() {
  TimerSensor.run();
  if (FlowSwitchStatus) {
    Serial.println('1');
  } else {
    Serial.println('0');
  }
}