// Line 463

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
// const char* ssid = "NT-HUAWEI-2.4G";
// const char* pass = "kai.kai.kai";
// const char* ssid = "Watnoo";
// const char* pass = "0817927275";
const char* ssid = "Inch";
const char* pass = "licht4825!";

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
// byte StatusSendLinePump = 0;
// byte StatusSendLineNotFlow = 0;
byte StatusSendLineHaveProblem = 0;
float current = 0;
// byte page = 0;


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
  // if (LineNoti) LINE.notify((String) "Received DelayFlow: " + DelayFlow + " sec");
  DelayFlow = DelayFlow * 1000;
  // if (page == 1) PageDelayFlow();
  // Serial.println((String) "Received DelayFlow: " + DelayFlow);
}

BLYNK_WRITE(V9) {
  AmpLimit = param.asInt();
  EEPROM.write(1, AmpLimit);
  EEPROM.commit();
  // if (page == 3) PageAmpLimit();
  // Serial.println((String) "Received AmpLimit: " + AmpLimit);
  // if (LineNoti) LINE.notify((String) "Received AmpLimit: " + AmpLimit + " Amp");
}

BLYNK_WRITE(V6) {
  DelayAmp = param.asInt();
  EEPROM.write(2, DelayAmp);
  EEPROM.commit();
  // if (LineNoti) LINE.notify((String) "Received DelayAmp: " + DelayAmp + " sec");
  DelayAmp = DelayAmp * 1000;
  // if (page == 2) PageDelayAmp();
  // Serial.println((String) "Received DelayAmp: " + DelayAmp);
}

BLYNK_WRITE(V0) {
  LineNoti = param.asInt();
  EEPROM.write(3, LineNoti);
  EEPROM.commit();
  String message = (String) "Received LineNoti: " + (LineNoti ? "True" : "False");
  // Serial.println(message);
  LINE.notify(message);
  // if (page == 4) PageToggle();
}

BLYNK_WRITE(V7) {
  ProtectMode = param.asInt();
  flowlasttime = lowamplasttime = millis();
  isExit = 0;
  EEPROM.write(4, ProtectMode);
  EEPROM.commit();
  String message = (String) "Received ProtectMode: " + (ProtectMode ? "True" : "False");
  // Serial.println(message);
  if (LineNoti) LINE.notify(message);
  // if (page == 4) PageToggle();
}

BLYNK_WRITE(V11) {
  int pinValue = param.asInt();
  if (pinValue) {
    isExit = 0;
    LEDabnormalcurrent.off();
    LEDforcestop.off();
    // Serial.println("isExit = 0");
    // if (LineNoti) LINE.notify("Reset System");
  }
}



void setup() {
  // Serial.begin(115200);
  WiFi.begin(ssid, pass);
  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.println("Connecting...");
  //   delay(250);
  // }
  Blynk.begin(auth, ssid, pass, "blynk.en-26.com", 9600);
  // Serial.println(LINE.getVersion());

  TimerClockDisplay.setInterval(850L, ClockDisplay);
  TimerSensor.setInterval(1050L, ReadSensor);


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
  // page = 0;

  Blynk.virtualWrite(V10, EEPROM.read(0));
  Blynk.virtualWrite(V9, EEPROM.read(1));
  Blynk.virtualWrite(V6, EEPROM.read(2));
  Blynk.virtualWrite(V0, EEPROM.read(3));
  Blynk.virtualWrite(V7, EEPROM.read(4));

  LINE.setToken(LToken);
  if (LineNoti) LINE.notify("Starting System");

  pinMode(FlowSwitchAt, INPUT);
  pinMode(RelayAt, OUTPUT);

  flowlasttime = lowamplasttime = millis();
  digitalWrite(RelayAt, 1);

  if (isnan(pzem.current())) current = 0;
  else current = pzem.current();

  // MainMenu();
  MainMenu1Page();
}



void ClockDisplay() {
  char Time[10];
  sprintf(Time, "%d:%02d:%02d", hour(), minute(), second());
  String Date = String(day()) + "/" + month() + "/" + year();
  // Serial.println((String) "Current time: " + Time + " " + Date);

  lcd.setCursor(20 - strlen(Time), 3);
  lcd.print(Time);

  Blynk.virtualWrite(V13, Time);
  Blynk.virtualWrite(V12, Date);
}



void ReadSensor() {
  if (isnan(pzem.current())) current = 0.00;
  else current = pzem.current();
  FlowSwitchStatus = digitalRead(FlowSwitchAt);
  lcd.setCursor(0, 3);
  lcd.print((String) "Amp:"+current+" A");
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
    // if (LineNoti) {
    //   if (!StatusSendLinePump) {
    //     LINE.notify("Water Pump Active");
    //     StatusSendLinePump = 1;
    //   }
    // }

    // Check FlowSwitch Status
    if (FlowSwitchStatus) {
      LEDFlowSwitch.on();  // on LED v1
      flowlasttime = millis();
    } else {
      LEDFlowSwitch.off();  // off LED v1
      if (millis() - flowlasttime > DelayFlow) {
        StatusSendLineHaveProblem = (isExit ? 0 : 1);
        isExit = 1;
      }
    }

    // ตรวจสอบกระแสผิดปกติ
    if (current > AmpLimit) {
      LEDabnormalcurrent.on();  // on LED v5
      if (millis() - lowamplasttime > DelayAmp) {
        StatusSendLineHaveProblem = (isExit ? 0 : 2);
        isExit = 2;
      }
    } else {
      LEDabnormalcurrent.off();  // off LED v5
      lowamplasttime = millis();
    }

  } else {
    LEDpumpworking.off();  // off LED v2
    LEDpumppause.on();     // on LED v3
    LEDFlowSwitch.off();
    LEDabnormalcurrent.off();
    flowlasttime = lowamplasttime = millis();
  }

  if (isExit == 1) {
    if (ProtectMode) {
      if (millis() - flowlasttime > DelayFlow) {
        digitalWrite(RelayAt, 0);
        LEDpumpworking.off();
        LEDpumppause.on();
      }
    }
    if (LineNoti && StatusSendLineHaveProblem) {
      LINE.notify("Having Problem (Water not flow)");
      StatusSendLineHaveProblem = 0;
    }
    LEDFlowSwitch.on();
    LEDforcestop.on();
  } else if (isExit == 2) {
    if (ProtectMode) {
      if (millis() - lowamplasttime > DelayAmp) {
        digitalWrite(RelayAt, 0);
        LEDpumpworking.off();
        LEDpumppause.on();
      }
    }
    if (LineNoti && StatusSendLineHaveProblem) {
      LINE.notify("Having Problem (High Amp usage)");
      StatusSendLineHaveProblem = 0;
    }
    LEDforcestop.on();
    LEDabnormalcurrent.on();
  } else {
    LEDforcestop.off();
    digitalWrite(RelayAt, 1);
  }
}



void MainMenu1Page() {
  String FD = (String) "FD:" + DelayFlow / 1000 + " s";
  String AD = (String) "AD:" + DelayAmp / 1000 + " s";
  String AL = (String) "AL:" + AmpLimit + " A";
  String Line = (String) "Line   :" + (LineNoti ? "On " : "Off");
  String Protect = (String) "Protect:" + (ProtectMode ? "On " : "Off");
  lcd.setCursor(0, 0);
  lcd.print(FD);
  lcd.setCursor(0, 1);
  lcd.print(AD);
  lcd.setCursor(0, 2);
  lcd.print(AL);
  lcd.setCursor(9, 0);
  lcd.print(Line);
  lcd.setCursor(9, 1);
  lcd.print(Protect);
}


/*
unsigned long DelayFlow = 0;
unsigned long AmpLimit = 0;
unsigned long DelayAmp = 0;
byte LineNoti = 0;
byte ProtectMode = 0;
*/


/*
Start MultiPage

void MainMenu() {
  page = 0;
  lcd.setCursor(0, 0);
  lcd.print("1:FlowDelay  4:Other");
  lcd.setCursor(0, 1);
  lcd.print("2:AmpDelay          ");
  lcd.setCursor(0, 2);
  lcd.print("3:AmpLimit          ");
}


void PageDelayFlow() {
  page = 1;
  lcd.setCursor(0, 0);
  lcd.print("FlowDelay :         ");
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
void PageDelayAmp() {
  page = 2;
  lcd.setCursor(0, 0);
  lcd.print("AmpDelay :          ");
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
void PageAmpLimit() {
  page = 3;
  lcd.setCursor(0, 0);
  lcd.print("AmpLimit :          ");
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
void PageToggle() {
  page = 4;
  lcd.setCursor(0, 0);
  lcd.print("A:Line Notify  : ");
  lcd.setCursor(17, 0);
  lcd.print(LineNoti ? "On " : "Off");
  lcd.setCursor(0, 1);
  lcd.print("B:Protect Mode : ");
  lcd.setCursor(17, 1);
  lcd.print(ProtectMode ? "On " : "Off");
  lcd.setCursor(0, 2);
  lcd.print("              D:Back");
}
*/

void UpdateDelayFlow(unsigned long value) {
  // PageDelayFlow();
  MainMenu1Page();
    Blynk.virtualWrite(V10, value);
  UpdateEEPROM(0, value, "DelayFlow");
  // Serial.println((String) "Received DelayFlow: " + value * 1000);
  // if (LineNoti) LINE.notify((String) "Received DelayFlow: " + value + " sec");
}
void UpdateDelayAmp(unsigned long value) {
  // PageDelayAmp();
  MainMenu1Page();
    Blynk.virtualWrite(V6, value);
  UpdateEEPROM(2, value, "DelayAmp");
  // Serial.println((String) "Received DelayAmp: " + value * 1000);
  // if (LineNoti) LINE.notify((String) "Received DelayAmp: " + value + " sec");
}
void UpdateAmpLimit(unsigned long value) {
  // PageAmpLimit();
  MainMenu1Page();
    Blynk.virtualWrite(V9, value);
  UpdateEEPROM(1, value, "AmpLimit");
  // Serial.println((String) "Received AmpLimit: " + value);
  // if (LineNoti) LINE.notify((String) "Received AmpLimit: " + value + " Amp");
}
void UpdatePageToggleLineNoti(byte value) {
  // PageToggle();
  MainMenu1Page();
    Blynk.virtualWrite(V0, value);
  UpdateEEPROM(3, value, "LineNoti");
  // String message = (String) "Received LineNoti: " + (LineNoti ? "True" : "False");
  // LINE.notify(message);
  // Serial.println(message);
}
void UpdatePageToggleProtectMode(byte value) {
  // PageToggle();
  MainMenu1Page();
    Blynk.virtualWrite(V7, value);
  UpdateEEPROM(4, value, "ProtectMode");
  // String message = (String) "Received ProtectMode: " + (ProtectMode ? "True" : "False");
  // if (LineNoti) LINE.notify(message);
  // Serial.println(message);
}

// End MultiPage



void UpdateEEPROM(byte address, unsigned long value, String label) {
  EEPROM.write(address, value);
  EEPROM.commit();
}



void loop() {
  char key = keypad.getKey();

  Blynk.run();
  TimerClockDisplay.run();
  TimerSensor.run();
  if (key) {
    // Start Show LCD

    switch (key) {
      case '1':
        DelayFlow += 1000;
        UpdateDelayFlow(DelayFlow / 1000);
        break;
      case '4':
        DelayFlow -= 1000;
        UpdateDelayFlow(DelayFlow / 1000);
        break;
      case '2':
        DelayAmp += 1000;
        UpdateDelayAmp(DelayAmp / 1000);
        break;
      case '5':
        DelayAmp -= 1000;
        UpdateDelayAmp(DelayAmp / 1000);
        break;
      case '3':
        AmpLimit++;
        UpdateAmpLimit(AmpLimit);
        break;
      case '6':
        AmpLimit--;
        UpdateAmpLimit(AmpLimit);
        break;
      case '*':
        LineNoti = (LineNoti ? 0 : 1);
        UpdatePageToggleLineNoti(LineNoti);
        break;
      case '#':
        ProtectMode = (ProtectMode ? 0 : 1);
        UpdatePageToggleProtectMode(ProtectMode);
        break;
    }

    // Start MiltiPage
    /*
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
      case 4:
        switch (key) {
          case 'A':
            LineNoti = (LineNoti ? 0 : 1);
            UpdatePageToggleLineNoti(LineNoti);
            break;
          case 'B':
            ProtectMode = (ProtectMode ? 0 : 1);
            UpdatePageToggleProtectMode(ProtectMode);
            break;
          case 'D': MainMenu(); break;
        }
    }
    */
    // End MultiPage
  }
  // End Show LCD
}