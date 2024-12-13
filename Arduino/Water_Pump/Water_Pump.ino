// ทำแจ้งเตือนไลน์ต่อ

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
const char* ssid = "Tenda_F1B010";
const char* pass = "kai.kai.kai";
// const char* ssid = "Inch";
// const char* pass = "licht4825!";

WidgetRTC rtc;
PZEM004Tv30 pzem(D6, D7);
BlynkTimer Timer, TimerSensor;
//LiquidCrystal_I2C lcd(0x27, 16, 2);

BLYNK_CONNECTED() {
  rtc.begin();
}

byte PumpStatus = 0;
byte RelayAt = D4;
byte FlowSwitchAt = D5;
unsigned long flowlasttime;
unsigned long lowamplasttime;
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
WidgetLED LEDforcestop(V8);

unsigned long Delayflow = 0;
unsigned long AmpLimit = 0;
unsigned long DelayAmp = 0;
byte LineNoti = 0;
byte ProtectMode = 0;


BLYNK_WRITE(V10) {
  Delayflow = param.asInt();
  EEPROM.write(0, Delayflow);
  EEPROM.commit();
  Delayflow = Delayflow * 1000;
  Serial.println((String) "Received Delayflow: " + Delayflow);
  if (LineNoti) LINE.notify((String) "Received Delayflow: " + Delayflow / 1000 + " sec");
}

BLYNK_WRITE(V9) {
  AmpLimit = param.asInt();
  EEPROM.write(1, AmpLimit);
  EEPROM.commit();
  Serial.println((String) "Received AmpLimit: " + AmpLimit);
  if (LineNoti) LINE.notify((String) "Received AmpLimit: " + AmpLimit + " Amp");
}

BLYNK_WRITE(V6) {
  DelayAmp = param.asInt();
  EEPROM.write(2, DelayAmp);
  EEPROM.commit();
  DelayAmp = DelayAmp * 1000;
  Serial.println((String) "Received DelayAmp: " + DelayAmp);
  if (LineNoti) LINE.notify((String) "Received DelayAmp: " + DelayAmp / 1000 + " sec");
}

BLYNK_WRITE(V0) {
  LineNoti = param.asInt();
  EEPROM.write(3, LineNoti);
  EEPROM.commit();
  Serial.println((String) "Received LineNoti: " + LineNoti);
  if (LineNoti) LINE.notify("Received LineNoti: True");
  else LINE.notify("Received LineNoti: False");
}

BLYNK_WRITE(V7) {
  ProtectMode = param.asInt();
  EEPROM.write(4, ProtectMode);
  EEPROM.commit();
  Serial.println((String) "Received ProtectMode: " + ProtectMode);
  if (LineNoti) {
    if (ProtectMode) LINE.notify("Received ProtectMode: True");
    else LINE.notify("Received ProtectMode: False");
  }
}

BLYNK_WRITE(V11) {
  int pinValue = param.asInt();
  if (pinValue) {
    isExit = 0;
    LEDabnormalcurrent.off();
    LEDforcestop.off();
    Serial.println("isExit = 0");
    if (LineNoti) LINE.notify("Reset System");
  }
}



void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
    delay(500);
  }
  Blynk.begin(auth, ssid, pass, "blynk.en-26.com", 9600);
  Serial.println(LINE.getVersion());

  Timer.setInterval(900L, clockDisplay);
  TimerSensor.setInterval(1000L, ReadSensor);


  EEPROM.begin(512);
  clockDisplay();

  Delayflow = EEPROM.read(0) * 1000;
  AmpLimit = EEPROM.read(1);
  DelayAmp = EEPROM.read(2) * 1000;
  LineNoti = EEPROM.read(3);
  ProtectMode = EEPROM.read(4);
  isExit = 0;

  Blynk.virtualWrite(V10, EEPROM.read(0));
  Blynk.virtualWrite(V9, EEPROM.read(1));
  Blynk.virtualWrite(V6, EEPROM.read(2));
  Blynk.virtualWrite(V0, EEPROM.read(3));
  Blynk.virtualWrite(V7, EEPROM.read(4));

  LINE.setToken(LToken);
  if (LineNoti) LINE.notify("Starting System");

  pinMode(FlowSwitchAt, INPUT);
  pinMode(RelayAt, OUTPUT);
  //  lcd.init();
  //  lcd.display();
  //  lcd.backlight();
  //  lcd.clear();
  //  lcd.print("Pump Status: ");
  flowlasttime = millis();
  lowamplasttime = millis();
  digitalWrite(RelayAt, 1);

  if (isnan(pzem.current())) current = 0;
  else current = pzem.current();
}



void clockDisplay() {
  char Time[10];
  sprintf(Time, "%d:%02d:%02d", hour(), minute(), second());
  String Date = String(day()) + "/" + month() + "/" + year();
  Serial.println((String) "Current time: " + Time + " " + Date);
  Serial.println(current);
  Blynk.virtualWrite(V13, Time);
  Blynk.virtualWrite(V12, Date);
}



void ReadSensor() {
  if (isnan(pzem.current())) current = 0;
  else current = pzem.current();
  FlowSwitchStatus = digitalRead(FlowSwitchAt);
}



void loop() {
  Blynk.run();
  Timer.run();
  TimerSensor.run();
  Blynk.virtualWrite(V4, current);

  // Check Pump Status
  if (current > 0.5) {    // ถ้าปั๊มทำงาน
    LEDpumpworking.on();  // เปิด LED v2
    LEDpumppause.off();   // ปิด LED v3
    if (LineNoti) {
      if (!StatusSendLinePump) {
        LINE.notify("Water Pump Active");
        StatusSendLinePump = 1;
      }
    }

    // Check FlowSwitch Status
    if (FlowSwitchStatus) {
      LEDFlowSwitch.on();  // on LED v1
      flowlasttime = millis();
    } else {
      LEDFlowSwitch.off();  // off LED v1
      if (millis() - flowlasttime > Delayflow) {
        isExit = 1;
      }
    }

    // ตรวจสอบกระแสผิดปกติ
    if (current > AmpLimit) {
      LEDabnormalcurrent.on();  // on LED v5
      if (millis() - lowamplasttime > DelayAmp) {
        isExit = 1;
      }
    } else {
      LEDabnormalcurrent.off();  // off LED v5
      lowamplasttime = millis();
    }

  } else {
    LEDpumpworking.off();  // off LED v2
    LEDpumppause.on();     // on LED v3
    flowlasttime = millis();
  }

  if (isExit) {
    if (ProtectMode) {
      digitalWrite(RelayAt, 0);
    }
    LEDforcestop.on();
  } else {
    LEDforcestop.off();
    digitalWrite(RelayAt, 1);
  }

  delay(100);
}