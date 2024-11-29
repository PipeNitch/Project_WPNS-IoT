#include <PZEM004Tv30.h>

PZEM004Tv30 pzem(D6, D7);

void setup() {
  Serial.begin(115200);
}

void loop() {
    float voltage = pzem.voltage();
    if(voltage != NAN){
        Serial.print("Voltage: "); Serial.print(pzem.voltage()); Serial.println("V");
    } else {
        Serial.println("Error reading voltage");
    }

    float current = pzem.current();
    if(current != NAN){
        Serial.print("Current: "); Serial.print(pzem.current()); Serial.println("A");
    } else {
        Serial.println("Error reading current");
    }

    Serial.println();
    delay(2000);
}
