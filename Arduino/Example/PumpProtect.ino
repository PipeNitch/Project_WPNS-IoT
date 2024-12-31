#include <ESP8266WiFi.h>
#include <TridentTD_LineNotify.h>
#include <BlynkSimpleEsp8266.h>
#include <EEPROM.h>
#include <WidgetRTC.h>
#include <Wire.h>

char ssid[] = "Tenda_F1B010";
char pass[] = "kai.kai.kai";

char auth[] = "-aeXPhLO_-lEV6PPQNU7HU1pJ_iWjXwm";
#define LINE_TOKEN "DUUj6o9CC9LZdRd0yyLFEgTa3kcqPOkhp4rbXxqG1vD"

#define PumPin D0
#define FlowSwitchPin D2
unsigned long Last_time;

WidgetRTC rtc;
BlynkTimer TimerClockDisplay;

BLYNK_CONNECTED() {
  rtc.begin();
}

byte PumStatus = 0;
byte FlowSwitchStatus = 0;
byte StatusSendLinePump = 0;
byte StatusSendLineNotFlow = 0;
byte StatusSendLineWaterFlow = 0;

unsigned long PeriodOff = 0;
byte LineOnOff = 0;


WidgetLED BlynkLEDAlert(V2);
WidgetLED BlynkLEDFlow(V4);

BLYNK_WRITE(V0) {
  PeriodOff = param.asInt();
  EEPROM.write(0, PeriodOff);
  EEPROM.commit();
  PeriodOff = PeriodOff * 1000;
  Serial.print("Received PeriodOff: ");
  Serial.println(PeriodOff);
}


BLYNK_WRITE(V3) {
  LineOnOff = param.asInt();
  EEPROM.write(1, LineOnOff);
  EEPROM.commit();
  Serial.println(LineOnOff);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  Serial.printf("WiFi connecting to %s\n",  ssid);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Blynk.begin(auth, ssid, pass, "blynk.en-26.com", 9600);
  Serial.println(LINE.getVersion());
  LINE.setToken(LINE_TOKEN);

  setSyncInterval(5 * 60);

  TimerClockDisplay.setInterval(900L, ClockDisplay); //ตั้งเวลา เรียกคำสั่ง 900 มิลลิวินาที ต่อครั้ง

  EEPROM.begin(512);
  ClockDisplay();
  LINE.notify("ระบบเริ่มทำงาน");
}// End of Setup

void ClockDisplay() {
  char Time[10];
  char date[11];

  sprintf (date, "%02d/%02d/%04d", day(), month(), year() + 543);
  sprintf(Time, "%02d:%02d:%02d", hour(), minute(), second());
  Blynk.virtualWrite(V12, String(date));
  Blynk.virtualWrite(V13, String(Time));
  //Serial.println(String(date));
  //Serial.println(String(Time));
}

void loop() {
  Blynk.run();
  TimerClockDisplay.run();
}
