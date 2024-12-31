int x = 1000;
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(x);
  digitalWrite(LED_BUILTIN, LOW);
  delay(x);
}
