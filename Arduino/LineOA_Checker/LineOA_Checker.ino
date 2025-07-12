#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "NT-HUAWEI-2.4G";
const char* password = "kai.kai.kai";

const char* host = "api.line.me";
const int httpsPort = 443;

String accessToken = "Bearer RUf7XUVWPTAghRrTQJ6G1quUQDh1v35uLrxR+fBMuzmQ5U4YNwvt7B8YVT4vy6+nXnrRl7RNOYu9GkTUowFyIxmBbSEQh3vMa7HmaWmPblJmhJ52lLRKD67Ph5vXDJ8qx74M9E6T8WT9FsD6kZATyAdB04t89/1O/w1cDnyilFU=";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  WiFiClientSecure client;
  client.setInsecure();

  // Serial.println("Connecting to LINE API...");

  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return;
  }

  String url = "/v2/bot/message/quota/consumption";

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Authorization: " + accessToken + "\r\n" +
               "Connection: close\r\n\r\n");

  String payload = "";
  bool isBody = false;

  while (client.connected() || client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      isBody = true;
      continue;
    }
    if (isBody) {
      payload += line + "\n";
    }
  }

  // Serial.println("JSON Body:");
  // Serial.println(payload);
  // Serial.print("Payload length: ");
  // Serial.println(payload.length());

  if (payload.length() == 0) {
    Serial.println("No JSON data received.");
    return;
  }

  StaticJsonDocument<64> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  int totalUsage = doc["totalUsage"];
  Serial.print("Total Usage: ");
  Serial.println(totalUsage);
}

void loop() {
  
}
