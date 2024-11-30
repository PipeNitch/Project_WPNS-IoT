#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TridentTD_LineNotify.h>
#include <WidgetRTC.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>


//----- UltraSonic ------
#define TRIG_PIN  D7
#define ECHO_PIN  D5
//#define ECHO_PIN  D8

#define Buzzer_Pin D0

//------ NeoPixel -------
#define NeoPixel1PIN D4
#define NeoPixel2PIN D8
#define NUMPIXELS1 38
#define NUMPIXELS2 5
Adafruit_NeoPixel pixels1 = Adafruit_NeoPixel(NUMPIXELS1, NeoPixel1PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUMPIXELS2, NeoPixel2PIN, NEO_GRB + NEO_KHZ800);

//===== Wifi =====
//char ssid[] = "Tenda_F1B010";
//char pass[] = "kai.kai.kai";

//char ssid[] = "ProjectDeep";
//char pass[] = "12345678";

char ssid[] = "Alpha1";
char pass[] = "12345678";

//char auth[] = "16-WLVy7BiPO21FIO2niSkQxEGg_Y-sm";
char auth[] = "-aeXPhLO_-lEV6PPQNU7HU1pJ_iWjXwm";
//#define LINE_TOKEN  "URaq5XsvWxebPq3bxQfSxzYOCwiXir4DvXqoLRq2gAk"
#define LINE_TOKEN  "DUUj6o9CC9LZdRd0yyLFEgTa3kcqPOkhp4rbXxqG1vD"

byte LineOnOff;
byte StatusSendLineRed = 0;
byte StatusSendLineYellow = 0;
byte StatusMode = 0;
byte RedToggle = 0;
byte Start = 0;

long duration;
int Distance;
int Deep;
int WaterLevel;
int NormalLevel, WatchLevel, WarnLevel;

WidgetLED BlynkLedGreen(V1);
WidgetLED BlynkLedYellow(V2);
WidgetLED BlynkLedRed(V3);
WidgetLED BlynkLedAuto(V21);
WidgetLED BlynkLedManual(V22);
WidgetLED BlynkLedWarn(V11);
WidgetRTC rtc;

LiquidCrystal_I2C lcd(0x27, 20, 4);

BlynkTimer TimerClockDisplay, TimerReadSensor , TimerCheckIf;

BLYNK_CONNECTED() {
  rtc.begin();
  //Blynk.syncVirtual(V7, V10, V9, V8);
  Blynk.syncVirtual(V5);
}

BLYNK_WRITE(V14) {
  if (StatusMode == 2) {
    StatusMode = 1;
    EEPROM.put(0, StatusMode);
    EEPROM.commit();
    BlynkLedAuto.on();
    BlynkLedManual.off();
    Serial.println("Auto");
    if (LineOnOff == 1) {
      LINE.notify("ตั้งค่าการแจ้งเตือนเป็น Auto");
    }
  }
}

BLYNK_WRITE(V15) {
  if (StatusMode == 1) {
    StatusMode = 2;
    EEPROM.put(0, StatusMode);
    EEPROM.commit();
    BlynkLedAuto.off();
    BlynkLedManual.on();
    Serial.println("Manual");
    if (LineOnOff == 1) {
      LINE.notify("ตั้งค่าการแจ้งเตือนเป็น Manual");
    }
  }
}

BLYNK_WRITE(V16) {
  String TextLine;
  float FloatWaterLevel;
  FloatWaterLevel = WaterLevel / 100.00;

  TextLine = String("ระดับน้ำขณะนี้💦 ") + WaterLevel +  String(" ซม. หรือ💦  ") +  FloatWaterLevel +  String(" เมตร") ;
  Serial.println(FloatWaterLevel);
  Serial.println(TextLine);
  LINE.notify(TextLine);
}

BLYNK_WRITE(V5) {
  LineOnOff = param.asInt();
  Serial.print("LineOnOff = ");
  Serial.println(LineOnOff);
  if (Start == 1) {
    if (LineOnOff == 1) {
      LINE.notify("🔉เปิด : การแจ้งข้อมูลระบบทางไลน์");
    } else {
      LINE.notify("🔇ปิด : การแจ้งข้อมูลระบบทางไลน์");
    }
  }
  Start = 1;
}

BLYNK_WRITE(V6) {
  if (StatusMode == 2) {
    String TextLine;
    float FloatWaterLevel;

    BlynkLedWarn.on();
    FloatWaterLevel = WaterLevel / 100.00;
    TextLine = String("🚨ระดับน้ำอยู่ในระดับอันตราย⚠ ") + WaterLevel +  String(" ซม. หรือ💦  ") +  FloatWaterLevel +  String(" เมตร") ;
    Serial.println(FloatWaterLevel);
    Serial.println(TextLine);
    LINE.notify(TextLine);
    BlynkLedWarn.off();
  }
}

BLYNK_WRITE(V7) {
  String TextLine;

  Deep = param.asInt();
  EEPROM.put(1, Deep);
  EEPROM.commit();
  TextLine = String("ตั้งค่าความลึก🌊 = ") + Deep +  String(" ซม.");
  if (LineOnOff == 1) {
    LINE.notify(TextLine);
  }
  lcd.setCursor(5, 3);
  lcd.print("    ");
  lcd.setCursor(5, 3);
  lcd.print(Deep);
}

BLYNK_WRITE(V8) {
  digitalWrite(Buzzer_Pin, HIGH);
}

BLYNK_WRITE(V9) {
  String TextLine;
  char TextWatchLevel[5];
  WatchLevel = param.asInt();
  EEPROM.put(10, WatchLevel);
  EEPROM.commit();
  TextLine = String("ค่าระดับเฝ้าระวัง😲 = ") + WatchLevel +  String(" ซม.");
  if (LineOnOff == 1) {
    LINE.notify(TextLine);
  }
}

BLYNK_WRITE(V10) {
  String TextLine;
  char TextWarnLevel[5];
  WarnLevel = param.asInt();
  EEPROM.put(15, WarnLevel);
  EEPROM.commit();
  TextLine = String("ค่าระดับแจ้งเตือน😱 = ") + WarnLevel +  String(" ซม.");
  if (LineOnOff == 1) {
    LINE.notify(TextLine);
  }
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  pixels1.begin();
  pixels2.begin();
  NeoPixelOFF();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(Buzzer_Pin , OUTPUT);
  digitalWrite(Buzzer_Pin, HIGH);

  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Flood warning system");
  lcd.setCursor(0, 1);
  lcd.print("Distance   :     cm.");
  lcd.setCursor(0, 2);
  lcd.print("Water Level:     cm.");
  lcd.setCursor(0, 3);
  lcd.print("Deep:    cm.");

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

  TimerReadSensor.setInterval(1000L, ReadSensor);
  TimerClockDisplay.setInterval(900L, ClockDisplay);
  //TimerCheckIf.setInterval(1200L, CheckIf);

  BlynkLedGreen.off();
  BlynkLedYellow.off();
  BlynkLedRed.off();
  BlynkLedAuto.off();
  BlynkLedManual.off();
  ClockDisplay();

  //======== EEPROM Read ========
  EEPROM.get(0, StatusMode);
  if (StatusMode == 1) {
    BlynkLedAuto.on();
  } else {
    BlynkLedManual.on();
  }

  EEPROM.get(1, Deep);
  Blynk.virtualWrite(V7, Deep);
  lcd.setCursor(5, 3);
  lcd.print("    ");
  lcd.setCursor(5, 3);
  lcd.print(Deep);

  EEPROM.get(5, NormalLevel);
  Blynk.virtualWrite(V8, NormalLevel);

  EEPROM.get(10, WatchLevel);
  Blynk.virtualWrite(V9, WatchLevel);

  EEPROM.get(15, WarnLevel);
  Blynk.virtualWrite(V10, WarnLevel);

  LINE.notify("เริ่มทำงาน");

}// End of Setup

void NeoPixelOFF() {
  for (int i = 0; i < NUMPIXELS1; i++) {
    pixels1.setPixelColor(i, pixels1.Color(0, 0, 0));
    pixels1.show();
  }
  for (int i = 0; i < NUMPIXELS2; i++) {
    pixels2.setPixelColor(i, pixels2.Color(0, 0, 0));
    pixels2.show();
  }
}

void NeoPixelRed() {
  for (int i = 0; i < NUMPIXELS1; i++) {
    pixels1.setPixelColor(i, pixels1.Color(255, 0, 0));
    pixels1.show();
  }
  for (int i = 0; i < NUMPIXELS2; i++) {
    pixels2.setPixelColor(i, pixels2.Color(255, 0, 0));
    pixels2.show();
  }
}

void NeoPixelYellow() {
  for (int i = 0; i < NUMPIXELS1; i++) {
    pixels1.setPixelColor(i, pixels1.Color(204, 204, 0));
    pixels1.show();
  }
  for (int i = 0; i < NUMPIXELS2; i++) {
    pixels2.setPixelColor(i, pixels2.Color(204, 204, 0));
    pixels2.show();
  }
}

void NeoPixelGreen() {
  for (int i = 0; i < NUMPIXELS1; i++) {
    pixels1.setPixelColor(i, pixels1.Color(0, 255, 0));
    pixels1.show();
  }
  for (int i = 0; i < NUMPIXELS2; i++) {
    pixels2.setPixelColor(i, pixels2.Color(0, 255, 0));
    pixels2.show();
  }
}

void ClockDisplay() {
  char Time[10], date[11];
  sprintf (date, "%02d/%02d/%04d", day(), month(), year() + 543);
  sprintf(Time, "%02d:%02d:%02d", hour(), minute(), second());
  lcd.setCursor(12, 3);
  lcd.print(Time);
  Blynk.virtualWrite(V13, String(Time));
  Blynk.virtualWrite(V12, String(date));
  //Serial.println(String(Time));
  //Serial.println(String(date));
}

void ReadSensor() {
  char TextDistance[5];
  char TextWaterLevel[5];

  // เคลียร์ Trig PIN
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // ส่งสัญญาณ Trig
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, LOW);

  // อ่านค่าสัญญาณ Echo
  duration = pulseIn(ECHO_PIN , HIGH);

  // ความเร็วเสียงในอากาศประมาณ 340 เมตร/วินาที หรือ 29 ไมโครวินาที/เซนติเมตร
  // ระยะทางที่ส่งเสียงออกไปจนเสียงสะท้อนกลับมาสามารถใช้หาระยะทางของวัตถุได้
  // เวลาที่ใช้คือ ระยะทางไปกลับ ดังนั้นระยะทางคือ ครึ่งหนึ่งของที่วัดได้

  Distance = duration / 29 / 2;
  WaterLevel = Deep - Distance;
  if (WaterLevel <= 0) {
    WaterLevel = 0;
  }

  sprintf(TextDistance, "%4d", Distance);
  sprintf(TextWaterLevel, "%4d", WaterLevel);
  lcd.setCursor(12, 1);
  lcd.print(TextDistance);
  lcd.setCursor(12, 2);
  lcd.print(TextWaterLevel);

  Serial.println(duration);
  Serial.print("Distance = ");
  Serial.print(Distance);
  Serial.println(" Cm");
  Blynk.virtualWrite(V0, Distance);
  Blynk.virtualWrite(V4, WaterLevel);
  CheckIf();
  
} // End of Read Sensor


void CheckIf() {
  if (WaterLevel < WatchLevel) {
    //ปกติ
    BlynkLedRed.off();
    BlynkLedYellow.off();
    BlynkLedGreen.on();
    NeoPixelGreen();
    StatusSendLineRed = 0;
    StatusSendLineYellow = 0;
    digitalWrite(Buzzer_Pin, HIGH);

  } else if ((WaterLevel >= WatchLevel) & (WaterLevel < WarnLevel)) {
    //เฝ้าระวัง
    BlynkLedRed.off();
    BlynkLedYellow.on();
    BlynkLedGreen.off();
    NeoPixelYellow();
    StatusSendLineRed=0;
    digitalWrite(Buzzer_Pin, HIGH);
    
    if (StatusMode == 1) {
      if (StatusSendLineYellow == 0) {
        String TextLine;
        float FloatWaterLevel;

        FloatWaterLevel = WaterLevel / 100.00;
        TextLine = String("🚨ระดับน้ำอยู่ในระดับ > เฝ้าระวัง") + WaterLevel +  String(" ซม. หรือ💦  ") +  FloatWaterLevel +  String(" เมตร") ;
        Serial.println(FloatWaterLevel);
        Serial.println(TextLine);
        LINE.notify(TextLine);
        StatusSendLineYellow = 1;
      }
    }
  } else {
    //แจ้งเตือน
    BlynkLedRed.on();
    BlynkLedYellow.off();
    BlynkLedGreen.off();
    StatusSendLineYellow=0;
    digitalWrite(Buzzer_Pin, LOW);
    if (RedToggle == 0) {
      NeoPixelRed();
      RedToggle = 1;
    } else {
      NeoPixelOFF();
      RedToggle = 0;
    }
    if (StatusMode == 1) {
      if (StatusSendLineRed == 0) {
        String TextLine;
        float FloatWaterLevel;

        BlynkLedWarn.on();
        FloatWaterLevel = WaterLevel / 100.00;
        TextLine = String("🚨ระดับน้ำอยู่ในระดับวิกฤต⚠ ") + WaterLevel +  String(" ซม. หรือ💦  ") +  FloatWaterLevel +  String(" เมตร") ;
        Serial.println(FloatWaterLevel);
        Serial.println(TextLine);
        LINE.notify(TextLine);
        StatusSendLineRed = 1;
        BlynkLedWarn.off();
      }
    }
  }
}

void loop()
{
  Blynk.run();
  TimerReadSensor.run();
  TimerClockDisplay.run();
  //TimerCheckIf.run();
}
