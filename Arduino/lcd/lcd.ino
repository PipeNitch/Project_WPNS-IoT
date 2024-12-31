#include <Wire.h> // เรียกใช้ไลบรารี Wire สำหรับการสื่อสารแบบ I2C
#include <LiquidCrystal_I2C.h> // เรียกใช้ไลบรารี LiquidCrystal_I2C สำหรับควบคุมจอ LCD ผ่าน I2C
// การสื่อสารแบบ I2C เป็นที่นิยมในการเชื่อมต่อกับอุปกรณ์ต่างๆ เช่น เซ็นเซอร์, จอ LCD เป็นต้น

LiquidCrystal_I2C lcd(0x27, 16, 2);
// 0x27 คือ address ของจอ LCD (อาจแตกต่างกันไปในแต่ละรุ่น)
// 16 คือจำนวนคอลัมน์ (ตัวอักษร) ในแต่ละแถวของจอ LCD
// 2 คือจำนวนแถวของจอ LCD

void setup() {
   lcd.begin(); // จอกว้าง 16 ตัวอักษร 2 บรรทัด
   lcd.display(); // ฟังก์ชันนี้ใช้สำหรับเปิดการแสดงผลบนจอ LCD
   lcd.backlight(); // ฟังก์ชันนี้ใช้สำหรับเปิดไฟ backlight ของจอ LCD
   lcd.clear(); // ฟังก์ชันนี้ใช้สำหรับล้างข้อมูลทั้งหมดบนจอ LCD

//  lcd.begin();
//  lcd.display();
//  lcd.backlight();
}

void loop() {
  for(int i=0;i<=9;i++){
    lcd.setCursor(0, 0); // เลื่อนเคอร์เซอร์ไปบรรทัดที่ 1 ลำดับที่ 1
    lcd.print(i); // สั่งให้แสดงผลค่า i ออกทางหน้าจอ lcd
    delay(100);
  }
}
#include <Wire.h> // เรียกใช้ไลบรารี Wire สำหรับการสื่อสารแบบ I2C
#include <LiquidCrystal_I2C.h> // เรียกใช้ไลบรารี LiquidCrystal_I2C สำหรับควบคุมจอ LCD ผ่าน I2C
// การสื่อสารแบบ I2C เป็นที่นิยมในการเชื่อมต่อกับอุปกรณ์ต่างๆ เช่น เซ็นเซอร์, จอ LCD เป็นต้น

LiquidCrystal_I2C lcd(0x27, 20, 4);
// 0x27 คือ address ของจอ LCD (อาจแตกต่างกันไปในแต่ละรุ่น)
// 16 คือจำนวนคอลัมน์ (ตัวอักษร) ในแต่ละแถวของจอ LCD
// 2 คือจำนวนแถวของจอ LCD

void setup() {
   lcd.init(); // จอกว้าง 16 ตัวอักษร 2 บรรทัด
   lcd.display(); // ฟังก์ชันนี้ใช้สำหรับเปิดการแสดงผลบนจอ LCD
   lcd.backlight(); // ฟังก์ชันนี้ใช้สำหรับเปิดไฟ backlight ของจอ LCD
   lcd.clear(); // ฟังก์ชันนี้ใช้สำหรับล้างข้อมูลทั้งหมดบนจอ LCD

//  lcd.begin();
//  lcd.display();
//  lcd.backlight();
}

void loop() {
  for(int i=0;i<=9;i++){
    lcd.setCursor(19, 3); // เลื่อนเคอร์เซอร์ไปบรรทัดที่ 1 ลำดับที่ 1
    lcd.print(i); // สั่งให้แสดงผลค่า i ออกทางหน้าจอ lcd
    delay(100);
  }
}
