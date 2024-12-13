#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WidgetRTC.h>
#include <Adafruit_Sensor.h>
#include <max6675.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad_I2C.h>
#include <TridentTD_LineNotify.h>
#include <TimeLib.h>
#include <EEPROM.h>
int Temp = 30;
int Tempnow;
int menuIndex = 0;
int lightActivationTimeHour = 0;
int lightActivationTimeMinute = 0;
int TimeBaked = 0;
long TimeNow = 0;

char auth[] = "tGC-pqGwDpadAS_2zkFIX2NeorEUIyLV";
char ssid[] = "Test";
char pass[] = "pppppppp";
//char ssid[] = "TP-Link_8206";
//char pass[] = "55521741";
char key;

byte TempMinHeater, TempMaxHeater, TempMinInfrared, TempMaxInfrared, TempMinFan, TempMaxFan, TempMinBlower, TempMaxBlower;

#define I2CADDR 0x21
#define LINE_TOKEN "BOaH5GSMgnd2Sm0ogSPl927Sz73xKKdB4GPaHe1MCkK"  // TOKEN
#define TIME_OFFSET 1 * 3600   // GMT+1 (สำหรับ GMT+7 เป็น 7 * 3600)
#define UPDATE_INTERVAL 60000  // อัพเดตทุก 1 นาที (60000 มิลลิวินาที)

unsigned long previousMillis = 0;
bool isCheckTimeFinishCalled = false;  // สร้างตัวแปรสถานะเพื่อตรวจสอบว่าฟังก์ชัน CheckTimeFinish ถูกเรียกหรือไม่

const long interval = 1000;  // ระยะเวลาที่ต้องการให้อ่านค่าทุก ๆ 1 วินาที
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

Keypad_I2C keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM, 0x20);
LiquidCrystal_I2C lcd(0x27, 20, 4);

byte HeaterPin = D0;
byte BlowerPin = D3;
byte InfraredPin = D7;
byte FanPin = D5;
byte StatusMode = 0;
BlynkTimer timer, TimerClockDisplay, TimerSensor, TimerLcdsen, TimerCheck, TimerCheckTimeFinish;

int thermoDO = D8;
int thermoCS = D6;
int thermoCLK = D4;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
WidgetRTC rtc;

WidgetLED BlynkHeater(V1);
WidgetLED BlynkBlower(V2);
WidgetLED BlynkInfrared(V3);
WidgetLED BlynkFan(V4);
WidgetLED BlynkLedAuto(V14);
WidgetLED BlynkLedManual(V13);

BLYNK_CONNECTED() {
  rtc.begin();
}

BLYNK_WRITE(V5) {

  if (StatusMode == 1) {
    ActiveHEATER();
  }
}

BLYNK_WRITE(V6) {
  if (StatusMode == 1) {
    ActiveBlower();
  }
}

BLYNK_WRITE(V7) {
  if (StatusMode == 1) {
    ActiveInfrared();
  }
}

BLYNK_WRITE(V8) {
  if (StatusMode == 1) {
    ActiveFan();
  }
}

BLYNK_WRITE(V10) {
  StatusMode = 1;
}

BLYNK_WRITE(V11) {
  StatusMode = 2;
}

BLYNK_WRITE(V12) {
  StatusMode = 0;
}

BLYNK_WRITE(V19) {  //TempMaxHEATER
  TempMaxHeater = param.asInt();
  EEPROM.write(1, TempMaxHeater);
  EEPROM.commit();
  Serial.println(EEPROM.read(1));
}

BLYNK_WRITE(V20) {  //TempMinHEATER
  TempMinHeater = param.asInt();
  EEPROM.write(2, TempMinHeater);
  EEPROM.commit();
  Serial.println(EEPROM.read(2));
}

BLYNK_WRITE(V21) {  //TempMaxInfrared
  TempMaxInfrared = param.asInt();
  EEPROM.write(3, TempMaxInfrared);
  EEPROM.commit();
  Serial.println(EEPROM.read(3));
}

BLYNK_WRITE(V22) {  //TempminInfrared
  TempMinInfrared = param.asInt();
  EEPROM.write(4, TempMinInfrared);
  EEPROM.commit();
  Serial.println(EEPROM.read(4));
}

BLYNK_WRITE(V24) {  //TempMaxBlower
  TempMaxBlower = param.asInt();
  EEPROM.write(5, TempMaxBlower);
  EEPROM.commit();
  Serial.println(EEPROM.read(5));
}

BLYNK_WRITE(V25) {  //TempMinBlower
  TempMinBlower = param.asInt();
  EEPROM.write(6, TempMinBlower);
  EEPROM.commit();
  Serial.println(EEPROM.read(6));
}

BLYNK_WRITE(V26) {  //TempMaxFan
  TempMaxFan = param.asInt();
  EEPROM.write(7, TempMaxFan);
  EEPROM.commit();
  Serial.println(EEPROM.read(7));
}

BLYNK_WRITE(V27) {  //TempMinFan
  TempMinFan = param.asInt();
  EEPROM.write(8, TempMinFan);
  EEPROM.commit();
  Serial.println(EEPROM.read(8));
}

BLYNK_WRITE(V23) {  //TimeBaked
  TimeBaked = param.asInt();
  EEPROM.write(9, TimeBaked);
  EEPROM.commit();
  Serial.println(EEPROM.read(9));
  TimerCheckTimeFinish.setInterval((TimeBaked + 4) * 1000L, CheckTimeFinish);  // แปลงเป็น milliseconds และตั้งค่าให้ Timer
}

void ActiveHEATER() {
  byte HeaterStatus = digitalRead(HeaterPin);
  if (HeaterStatus) {
    digitalWrite(HeaterPin, LOW);
    lcd.setCursor(8, 1);
    lcd.print(" ");
    BlynkHeater.off();
    LINE.notify("ปิดฮีตเตอร์แล้ว");
  } else {
    digitalWrite(HeaterPin, HIGH);
    lcd.setCursor(8, 1);
    lcd.print("*");
    BlynkHeater.on();
    LINE.notify("เปิดฮีตเตอร์แล้ว");
  }
}

void ActiveBlower() {
  byte BlowerStatus = digitalRead(BlowerPin);
  if (BlowerStatus) {
    digitalWrite(BlowerPin, LOW);
    lcd.setCursor(19, 1);
    lcd.print(" ");
    BlynkBlower.off();
    LINE.notify("ปิดโบลเวอร์แล้ว");
  } else {
    digitalWrite(BlowerPin, HIGH);
    lcd.setCursor(19, 1);
    lcd.print("*");
    BlynkBlower.on();
    LINE.notify("เปิดโบลเวอร์แล้ว");
  }
}

void ActiveInfrared() {
  byte InfraredStatus = digitalRead(InfraredPin);
  if (InfraredStatus) {
    digitalWrite(InfraredPin, LOW);
    lcd.setCursor(10, 2);
    lcd.print(" ");
    BlynkInfrared.off();
    LINE.notify("ปิดอินฟาเรตแล้ว");
  } else {
    digitalWrite(InfraredPin, HIGH);
    lcd.setCursor(10, 2);
    lcd.print("*");
    BlynkInfrared.on();
    LINE.notify("เปิดอินฟาเรตแล้ว");
  }
}

void ActiveFan() {
  byte FanStatus = digitalRead(FanPin);
  if (FanStatus) {
    digitalWrite(FanPin, LOW);
    lcd.setCursor(16, 2);
    lcd.print(" ");
    BlynkFan.off();
    LINE.notify("ปิดพัดลมแล้ว");
  } else {
    digitalWrite(FanPin, HIGH);
    lcd.setCursor(16, 2);
    lcd.print("*");
    BlynkFan.on();
    LINE.notify("เปิดพัดลมแล้ว");
  }
}


void setup() {
  pinMode(HeaterPin, OUTPUT);
  pinMode(BlowerPin, OUTPUT);
  pinMode(InfraredPin, OUTPUT);
  pinMode(FanPin, OUTPUT);
  Blynk.begin(auth, ssid, pass, "blynk.en-26.com", 9600);
  Serial.println("Connecting Blynk : ");
  Wire.begin();
  keypad.begin(makeKeymap(keys));
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  Menu();

  Serial.println(LINE.getVersion());
  Serial.printf("WiFi connecting ", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());
  LINE.setToken(LINE_TOKEN);

  // ส่งข้อความ
  LINE.notify("แจ้งเตือน");
  LINE.notify("มีการใช้งาน");
  EEPROM.begin(512);
  setSyncInterval(5 * 60);
  TimerClockDisplay.setInterval(1000L, ClockDisplay);  //<--Timer 1 จับเวลา 1 วินาที ครบ 1 วินาทีไปทำใน void Timer1ClockDisplay
  TimerSensor.setInterval(4789L, ReadSensor);
  TimerCheck.setInterval(1454L, CheckIf);
  //TimerLcdsen.setInterval(300L, Menu);

  //TimerLcdsen.run();
  StatusMode = EEPROM.read(0);
  Blynk.virtualWrite(V19, EEPROM.read(1));
  Blynk.virtualWrite(V20, EEPROM.read(2));
  Blynk.virtualWrite(V21, EEPROM.read(3));
  Blynk.virtualWrite(V22, EEPROM.read(4));
  Blynk.virtualWrite(V24, EEPROM.read(6));
  Blynk.virtualWrite(V25, EEPROM.read(7));
  Blynk.virtualWrite(V26, EEPROM.read(8));
  Blynk.virtualWrite(V27, EEPROM.read(9));
  BlynkHeater.off();
  BlynkInfrared.off();
  BlynkLedAuto.off();
  BlynkLedManual.off();
  BlynkBlower.off();
  BlynkFan.off();
  /*
  if (StatusMode == 1) {
    Manual();
  } else if (StatusMode == 2) {
    Auto();
  }
  */
}  //End setup

void CheckIf() {
  //checkheater
  StatusMode = 2;
  int TempInt = Tempnow;

  if (TempInt < TempMinHeater) {
    digitalWrite(HeaterPin, HIGH);
    lcd.setCursor(7, 2);
    lcd.print("*");
    BlynkHeater.on();
  } else if (TempInt >= TempMaxHeater) {
    digitalWrite(HeaterPin, LOW);
    lcd.setCursor(7, 2);
    lcd.print(" ");
    BlynkHeater.off();
  }

  //checkinfarate

  if (TempInt < TempMinInfrared) {
    digitalWrite(InfraredPin, HIGH);
    lcd.setCursor(9, 2);
    lcd.print("*");
    BlynkInfrared.on();
  } else if (TempInt >= TempMaxInfrared) {
    digitalWrite(InfraredPin, LOW);
    lcd.setCursor(9, 2);
    lcd.print(" ");
    BlynkInfrared.off();
  }

  //checkBlower
  if (TempInt < TempMinBlower) {
    digitalWrite(BlowerPin, HIGH);
    lcd.setCursor(11, 2);
    lcd.print("*");
    BlynkBlower.on();
  } else if (TempInt >= TempMaxBlower) {
    digitalWrite(BlowerPin, LOW);
    lcd.setCursor(11, 2);
    lcd.print(" ");
    BlynkBlower.off();
  }

  //checkFan
  if (TempInt <= TempMinFan) {
    digitalWrite(FanPin, LOW);
    lcd.setCursor(13, 2);
    lcd.print(" ");
    BlynkFan.off();
  } else if (TempInt >= TempMaxFan) {
    digitalWrite(FanPin, HIGH);
    lcd.setCursor(13, 2);
    lcd.print("*");
    BlynkFan.on();
  }

}  // End CheckIf

void ReadSensor() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Tempnow = thermocouple.readCelsius();  // อ่านค่าอุณหภูมิ แทน int Tempnow = thermocouple.readCelsius();
    if (isnan(thermocouple.readCelsius())) {
      Serial.println("Failed to read from MAX6675 sensor!");
      while (1)
        ;
    }
    Serial.print("Temperature: ");
    Serial.println(Tempnow);
    // ส่งค่าอุณหภูมิไปยังแอปพลิเคชัน Blynk
    Blynk.virtualWrite(V16, Tempnow);
    Blynk.virtualWrite(V17, Tempnow);
    lcd.setCursor(16, 0);
    lcd.print(Tempnow);
    lcd.print("'C");
  }
}  //End ReadSensor

void Menu() {
  StatusMode = 0;
  EEPROM.write(0, StatusMode);
  EEPROM.commit();

  ClockDisplay();
  ReadSensor();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("A:Manual");
  lcd.setCursor(0, 2);
  lcd.print("B:Auto");
}  //End menu

void Manual() {
  StatusMode = 1;
  EEPROM.write(0, StatusMode);
  EEPROM.commit();

  lcd.clear();
  ClockDisplay();
  ReadSensor();
  lcd.setCursor(0, 0);
  lcd.print("Manual");
  lcd.setCursor(0, 1);
  lcd.print("1:HEATER");
  lcd.setCursor(11, 1);
  lcd.print("2:BLOWER");
  lcd.setCursor(0, 2);
  lcd.print("3:INFRARED");
  lcd.setCursor(11, 2);
  lcd.print("4:FAN");
  lcd.setCursor(0, 3);
  lcd.print("D.Back");
  BlynkLedManual.on();
  LINE.notify("เข้าโหมดManual");
  while (1) {
    Blynk.run();
    TimerSensor.run();
    TimerClockDisplay.run();
    char key = keypad.getKey();
    if (key) {
      Serial.println(key);
      if (key == 'D') {
        StatusMode = 0;
      } else if (key == '1') {
        ActiveHEATER();
      } else if (key == '2') {
        ActiveBlower();
      } else if (key == '3') {
        ActiveInfrared();
      } else if (key == '4') {
        ActiveFan();
      }
    }
    if (StatusMode == 0) {
      break;
    }
  }  //Endwhile

  lcd.setCursor(0, 2);
  lcd.print("      ");
  CloseAll();
  Menu();
  LINE.notify("ออกจากโหมดManual");
}  //End manual

void Auto() {
  StatusMode = 2;
  EEPROM.write(0, StatusMode);
  EEPROM.commit();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Auto Menu");
  lcd.setCursor(0, 3);
  lcd.print("D:Exit");
  BlynkLedAuto.on();
  ReadSensor();
  ClockDisplay();

  TempMaxHeater = EEPROM.read(1);
  TempMinHeater = EEPROM.read(2);
  TempMaxInfrared = EEPROM.read(3);
  TempMinInfrared = EEPROM.read(4);
  TempMaxBlower = EEPROM.read(5);
  TempMinBlower = EEPROM.read(6);
  TempMaxFan = EEPROM.read(7);
  TempMinFan = EEPROM.read(8);

  Blynk.virtualWrite(V19, EEPROM.read(1));
  Blynk.virtualWrite(V20, EEPROM.read(2));
  Blynk.virtualWrite(V21, EEPROM.read(3));
  Blynk.virtualWrite(V22, EEPROM.read(4));
  Blynk.virtualWrite(V24, EEPROM.read(6));
  Blynk.virtualWrite(V25, EEPROM.read(7));
  Blynk.virtualWrite(V26, EEPROM.read(8));
  Blynk.virtualWrite(V27, EEPROM.read(9));
  LINE.notify("เข้าโหมดAuto");
  
  while (1) {
    Blynk.run();
    TimerSensor.run();
    TimerClockDisplay.run();
    TimerCheck.run();
    TimerCheckTimeFinish.run();
    char key = keypad.getKey();
    if (key) {
      Serial.println(key);
      if (key == 'D') {
        StatusMode = 0;
      }
    }
    if (StatusMode == 0) {

      break;
    }
  }  // End while

  lcd.setCursor(11, 2);
  lcd.print("  ");
  lcd.setCursor(13, 2);
  lcd.print("  ");
  lcd.setCursor(0, 2);
  lcd.print("      ");
  CloseAll();
  Menu();
  LINE.notify("ออกจากโหมดAuto");
}  //End Auto

void ClockDisplay() {
  char Time[10];
  sprintf(Time, "%02d:%02d:%02d", hour(), minute(), second());
  lcd.setCursor(12, 3);
  lcd.print(Time);
  Serial.println(String(Time));

}  // end of void ClockDisplay

void CloseAll() {
  BlynkHeater.off();
  BlynkInfrared.off();
  BlynkBlower.off();
  BlynkFan.off();
  BlynkLedAuto.off();
  BlynkLedManual.off();
  digitalWrite(HeaterPin, LOW);
  digitalWrite(InfraredPin, LOW);
  digitalWrite(FanPin, LOW);
  digitalWrite(BlowerPin, LOW);
}  //End voidcloseAll

void CheckTimeFinish() {
  StatusMode = 0;
  TimerCheckTimeFinish.disableAll();
}

void loop() {
  Blynk.run();
  TimerClockDisplay.run();  // 1 sec.
  TimerSensor.run();        // 2 sec.
  //TimerCheck.run();

  char key = keypad.getKey();
  if (key) {
    Serial.println(key);
    if (key == 'A') {
      StatusMode = 1;
    } else if (key == 'B') {
      StatusMode = 2;
    }
  }

  if (StatusMode == 1) {
    Manual();
  } else if (StatusMode == 2) {
    Auto();
  }
}  //End loop


