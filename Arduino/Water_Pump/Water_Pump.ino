// ต่อบรรทัด 371

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

const char* auth = "81PnZkd7Wmmt2rGAmoO7NaqeZ-Jc8kNd";
const char* LToken = "cZW2GpTn64FenmiTt38AfgMHWvt4XdlLS7gqChQmwnO";

// only 2.4 G
// const char* ssid = "Tenda_F1B010";
// const char* pass = "kai.kai.kai";
const char* ssid = "Watnoo";
const char* pass = "0817927275";
// const char* ssid = "Inch";
// const char* pass = "licht4825!";

const byte ROW_NUM = 4;
const byte COLUMN_NUM = 4;

char keys[ROW_NUM][COLUMN_NUM] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte pin_rows[ROW_NUM] = { 0, 1, 2, 3 };
byte pin_column[COLUMN_NUM] = { 4, 5, 6, 7 };

WidgetRTC rtc;
PZEM004Tv30 pzem(D6, D7);
BlynkTimer TimerClockDisplay, TimerSensor;
Keypad_I2C keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM, 0x20);
LiquidCrystal_I2C lcd(0x27, 20, 4);

BLYNK_CONNECTED() {
  rtc.begin();
}


byte PumpStatus = 0;
byte RelayAt = D4;
byte FlowSwitchAt = D5;
unsigned long flowlasttime;
unsigned long lowamplasttime;
byte FlowSwitchStatus = 0;
// isExit = 1 ; เพราะ Flow Switch ไม่ทำงาน
// isExit = 2 ; เพราะ ไฟเกิน
byte isExit = 0;
byte StatusSendLinePump = 0;
// byte StatusSendLineNotFlow = 0;
// StatusSendLineForceStop = 1 ; เพราะ Flow Switch ไม่ทำงาน
byte StatusSendLineForceStop = 0;
float current = 0;
byte page = 0;


WidgetLED LEDpumpworking(V2);
WidgetLED LEDpumppause(V3);
WidgetLED LEDFlowSwitch(V1);
WidgetLED LEDabnormalcurrent(V5);
WidgetLED LEDforcestop(V8);

unsigned long DelayFlow = 0;
unsigned long AmpLimit = 0;
unsigned long DelayAmp = 0;
byte LineNoti = 0;
byte ProtectMode = 0;


BLYNK_WRITE(V10) {
  DelayFlow = param.asInt();
  EEPROM.write(0, DelayFlow);
  EEPROM.commit();
  DelayFlow = DelayFlow * 1000;
  if (page == 1) PageDelayFlow();
  Serial.println((String) "Received DelayFlow: " + DelayFlow);
  // if (LineNoti) LINE.notify((String) "Received DelayFlow: " + DelayFlow / 1000 + " sec");
}

BLYNK_WRITE(V9) {
  AmpLimit = param.asInt();
  EEPROM.write(1, AmpLimit);
  EEPROM.commit();
  if (page == 3) PageAmpLimit();
  Serial.println((String) "Received AmpLimit: " + AmpLimit);
  // if (LineNoti) LINE.notify((String) "Received AmpLimit: " + AmpLimit + " Amp");
}

BLYNK_WRITE(V6) {
  DelayAmp = param.asInt();
  EEPROM.write(2, DelayAmp);
  EEPROM.commit();
  DelayAmp = DelayAmp * 1000;
  if (page == 2) PageDelayAmp();
  Serial.println((String) "Received DelayAmp: " + DelayAmp);
  // if (LineNoti) LINE.notify((String) "Received DelayAmp: " + DelayAmp / 1000 + " sec");
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
    // if (LineNoti) LINE.notify("Reset System");
  }
}



void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.println("Connecting...");
  //   delay(250);
  // }
  Blynk.begin(auth, ssid, pass, "blynk.en-26.com", 9600);
  Serial.println(LINE.getVersion());

  TimerClockDisplay.setInterval(850L, ClockDisplay);
  TimerSensor.setInterval(950L, ReadSensor);


  keypad.begin(makeKeymap(keys));
  EEPROM.begin(512);
  ClockDisplay();
  lcd.init();
  lcd.backlight();
  lcd.clear();

  DelayFlow = EEPROM.read(0) * 1000;
  AmpLimit = EEPROM.read(1);
  DelayAmp = EEPROM.read(2) * 1000;
  LineNoti = EEPROM.read(3);
  ProtectMode = EEPROM.read(4);
  isExit = 0;
  page = 0;

  Blynk.virtualWrite(V10, EEPROM.read(0));
  Blynk.virtualWrite(V9, EEPROM.read(1));
  Blynk.virtualWrite(V6, EEPROM.read(2));
  Blynk.virtualWrite(V0, EEPROM.read(3));
  Blynk.virtualWrite(V7, EEPROM.read(4));

  LINE.setToken(LToken);
  if (LineNoti) LINE.notify("Starting System");

  pinMode(FlowSwitchAt, INPUT);
  pinMode(RelayAt, OUTPUT);

  flowlasttime = millis();
  lowamplasttime = millis();
  digitalWrite(RelayAt, 1);

  if (isnan(pzem.current())) current = 0;
  else current = pzem.current();

  MainMenu();
}



void ClockDisplay() {
  char Time[10];
  sprintf(Time, "%d:%02d:%02d", hour(), minute(), second());
  String Date = String(day()) + "/" + month() + "/" + year();
  Serial.println((String) "Current time: " + Time + " " + Date);
  // Serial.println(current);

  lcd.setCursor(20 - strlen(Time), 3);
  lcd.print(Time);

  Blynk.virtualWrite(V13, Time);
  Blynk.virtualWrite(V12, Date);
}



void ReadSensor() {
  if (isnan(pzem.current())) current = 0;
  else current = pzem.current();
  FlowSwitchStatus = digitalRead(FlowSwitchAt);
  // if (page == 0) {
  //   lcd.setCursor(0, 3);
  //   lcd.print("A : ");
  //   lcd.setCursor(4, 3);
  //   lcd.print(current);
  // }
  Blynk.virtualWrite(V4, current);

  // Start CheckIf
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
      if (millis() - flowlasttime > DelayFlow) {
        isExit = 1;
        StatusSendLineForceStop = 1;
      }
    }

    // ตรวจสอบกระแสผิดปกติ
    if (current > AmpLimit) {
      LEDabnormalcurrent.on();  // on LED v5
      if (millis() - lowamplasttime > DelayAmp) {
        isExit = 2;
        StatusSendLineForceStop = 2;
      }
    } else {
      LEDabnormalcurrent.off();  // off LED v5
      lowamplasttime = millis();
    }

  } else {
    LEDpumpworking.off();  // off LED v2
    LEDpumppause.on();     // on LED v3
    LEDFlowSwitch.off();
    flowlasttime = millis();
  }

  if (isExit == 1) {
    if (ProtectMode) {
      digitalWrite(RelayAt, 0);
    }
    LEDforcestop.on();
  } else if (isExit == 2) {
    if (ProtectMode) {
      digitalWrite(RelayAt, 0);
    }
    LEDforcestop.on();
    LEDabnormalcurrent.on();
  } else {
    LEDforcestop.off();
    LEDabnormalcurrent.off();
    digitalWrite(RelayAt, 1);
  }

  if (LineNoti) {
    if (StatusSendLineForceStop == 1) {
      LINE.notify("Force stop (Water not flow)");
      StatusSendLineForceStop = 0;
    } else if (StatusSendLineForceStop == 2) {
      LINE.notify("Force stop (High Amp usage)");
      StatusSendLineForceStop = 0;
    }
  }
  // End CheckIf
}



void MainMenu() {
  page = 0;
  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1:FlowDelay  4:Other");
  lcd.setCursor(0, 1);
  lcd.print("2:AmpDelay          ");
  lcd.setCursor(0, 2);
  lcd.print("3:AmpLimit          ");
}

void PageDelayFlow() {
  page = 1;
  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Flow Delay : ");
  lcd.setCursor(13, 0);
  lcd.print(DelayFlow / 1000);
  lcd.setCursor(18, 0);
  lcd.print("s");
  lcd.setCursor(0, 1);
  lcd.print("A:Add               ");
  lcd.setCursor(0, 2);
  lcd.print("B:Reduce      D:Back");
  // lcd.setCursor(14, 2);
  // lcd.print("D:Back");
}
void UpdateDelayFlow(unsigned long value) {
  Blynk.virtualWrite(V10, value);
  UpdateEEPROM(0, value, "DelayFlow");
  Serial.println((String) "Received DelayFlow: " + value * 1000);
  if (LineNoti) LINE.notify((String) "Received DelayFlow: " + value + " sec");
  PageDelayFlow();
}

void PageDelayAmp() {
  page = 2;
  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Amp Delay : ");
  lcd.setCursor(12, 0);
  lcd.print(DelayAmp / 1000);
  lcd.setCursor(18, 0);
  lcd.print("s");
  lcd.setCursor(0, 1);
  lcd.print("A:Add               ");
  lcd.setCursor(0, 2);
  lcd.print("B:Reduce      D:Back");
  // lcd.setCursor(14, 2);
  // lcd.print("D:Back");
}
void UpdateDelayAmp(unsigned long value) {
  Blynk.virtualWrite(V6, value);
  UpdateEEPROM(2, value, "DelayAmp");
  Serial.println((String) "Received DelayAmp: " + value * 1000);
  if (LineNoti) LINE.notify((String) "Received DelayAmp: " + value + " sec");
  PageDelayAmp();
}

void PageAmpLimit() {
  page = 3;
  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Amp Limit : ");
  lcd.setCursor(12, 0);
  lcd.print(AmpLimit);
  lcd.setCursor(18, 0);
  lcd.print("A");
  lcd.setCursor(0, 1);
  lcd.print("A:Add               ");
  lcd.setCursor(0, 2);
  lcd.print("B:Reduce      D:Back");
  // lcd.setCursor(14, 2);
  // lcd.print("D:Back");
}
void UpdateAmpLimit(unsigned long value) {
  Blynk.virtualWrite(V9, value);
  UpdateEEPROM(1, value, "AmpLimit");
  Serial.println((String) "Received AmpLimit: " + value);
  if (LineNoti) LINE.notify((String) "Received AmpLimit: " + value + " Amp");
  PageAmpLimit();
}

void PageToggle(){
  page = 4;
  lcd.setCursor(0, 0);
  lcd.print("Line Notify : ");
  lcd.setCursor(14, 0);
  lcd.print(LineNoti?"Yes":"No ");
  lcd.setCursor(0, 1);
  lcd.print("Protect Mode : ");
  lcd.setCursor(15, 1);
  lcd.print(ProtectMode?"Yes":"No");
}
void UpdatePageToggleLineNoti(){
  Blynk.virtualWrite()
}



void UpdateEEPROM(byte address, unsigned long value, String label) {
  EEPROM.write(address, value);
  EEPROM.commit();
  // Serial.println("commited EEPROM");
}



void loop() {
  char key = keypad.getKey();

  Blynk.run();
  TimerClockDisplay.run();
  TimerSensor.run();
  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);
    // Start Show LCD
    switch (page) {
      case 0:
        switch (key) {
          case '1': PageDelayFlow(); break;
          case '2': PageDelayAmp(); break;
          case '3': PageAmpLimit(); break;
          case '4': PageToggle(); break;
        }
        break;
      case 1:
        switch (key) {
          case 'A':
            DelayFlow += 1000;
            UpdateDelayFlow(DelayFlow / 1000);
            break;
          case 'B':
            DelayFlow -= 1000;
            UpdateDelayFlow(DelayFlow / 1000);
            break;
          case 'D': MainMenu(); break;
        }
        break;
      case 2:
        switch (key) {
          case 'A':
            DelayAmp += 1000;
            UpdateDelayAmp(DelayAmp / 1000);
            break;
          case 'B':
            DelayAmp -= 1000;
            UpdateDelayAmp(DelayAmp / 1000);
            break;
          case 'D': MainMenu(); break;
        }
        break;
      case 3:
        switch (key) {
          case 'A':
            AmpLimit++;
            UpdateAmpLimit(AmpLimit);
            break;
          case 'B':
            AmpLimit--;
            UpdateAmpLimit(AmpLimit);
            break;
          case 'D': MainMenu(); break;
        }
        break;
    }
  }
  // End Show LCD
}