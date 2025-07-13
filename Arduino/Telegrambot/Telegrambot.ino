#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "licht";
const char* password = "licht4825!";

#define BOTtoken "7796144445:AAGr5zH_RP1n7oc2ehF653BvTZrCtZ3dn2I"
#define CHAT_ID "-4973400356"

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

void setup() {
  Serial.begin(115200);

  configTime(0, 0, "pool.ntp.org");
  client.setTrustAnchors(&cert);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  bot.sendMessage(CHAT_ID, "📢 แจ้งเตือน: บอทเริ่มทำงานแล้ว!", "");
}

void loop() {
}