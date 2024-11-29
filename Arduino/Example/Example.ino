#include <ESP8266WiFi.h>
#include <TridentTD_LineNotify.h>
#include <BlynkSimpleEsp8266.h>
#include <EEPROM.h>
#include <WidgetRTC.h>

#define LINE_TOKEN ""

const char* auth[] = ""; 
const char* ssid[] = "";
const char* pass[] = "";

#define PumPin D0
#define FlowSwitchPin D2
unsigned long Last_time;

WidgetRTC ;
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
  Blynk.begin(auth, ssid, pass, "blynk.en-26.com", 9600);
  Serial.println(LINE.getVersion());
  LINE.setToken(LINE_TOKEN);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  EEPROM.begin(512);
  
  TimerClockDisplay.setInterval(800L, ClockDisplay);
  PeriodOff = EEPROM.read(0) * 1000;
  LineOnOff = EEPROM.read(1);
  
  pinMode(PumPin, INPUT);
  pinMode(FlowSwitchPin, INPUT);
  Last_time = millis();
  Blynk.virtualWrite(V0, EEPROM.read(0));
  Blynk.virtualWrite(V3, LineOnOff);
  LINE.setToken(LINE_TOKEN);
  LINE.notify("เริ่มทำงาน");
}



void ClockDisplay() {
  char Time[10];
  sprintf(Time, "%02d:%02d:%02d", hour(), minute(), second());
  Blynk.virtualWrite(V20, String(Time));
  Serial.println(String(Time));
}



void loop() {
  Blynk.run();
  TimerClockDisplay.run();
  
  PumStatus = digitalRead(PumPin);
  Serial.print("Pump Status: ");
  Serial.println(PumStatus);

  FlowSwitchStatus = digitalRead(FlowSwitchPin);
  Serial.print("FlowSwitch Status: ");
  Serial.println(FlowSwitchStatus);

  if (PumStatus == 0) {
    BlynkLEDAlert.on();
    Blynk.virtualWrite(V1, "เปิด");
    if (LineOnOff == 1) {
      if (StatusSendLinePump == 0) {
        LINE.notify("ปั๊มน้ำทำงาน");
        StatusSendLinePump = 1;
      }
    }
    if (FlowSwitchStatus == 1) {
      StatusSendLineWaterFlow = 0;
      if ( millis() - Last_time > PeriodOff) {
        BlynkLEDFlow.on();
        if (LineOnOff == 1) {
          if (StatusSendLineNotFlow == 0) {
            LINE.notify("ทำงาน น้ำไม่ไหล!!!!!");
            StatusSendLineNotFlow = 1;
          }
        }
      }
    } else {
      StatusSendLineNotFlow = 0;
      Last_time = millis();
      BlynkLEDFlow.off();
      if (LineOnOff == 1) {
        if (StatusSendLineWaterFlow == 0) {
          LINE.notify("ทำงาน น้ำไหล");
          StatusSendLineWaterFlow = 1;
        }
      }
    }
  } else {
    if (LineOnOff == 1) {
      if (StatusSendLinePump == 1) {
        LINE.notify("ปั๊มน้ำหยุดทำงาน");
        StatusSendLinePump = 0;
      }
    }
    Blynk.virtualWrite(V1, "ปิด");
    BlynkLEDAlert.off();
    Last_time = millis();
    StatusSendLinePump = 0;
    StatusSendLineNotFlow = 0;
    StatusSendLineWaterFlow = 0;
  }
  delay(1000);
}
