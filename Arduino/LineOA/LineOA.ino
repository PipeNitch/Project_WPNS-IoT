#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// const char* ssid = "licht";
// const char* password = "licht4825!";

const char* ssid = "NT-HUAWEI-2.4G";
const char* password = "kai.kai.kai";
const char* host = "api.line.me";
const int httpsPort = 443;

String accessToken = "RUf7XUVWPTAghRrTQJ6G1quUQDh1v35uLrxR+fBMuzmQ5U4YNwvt7B8YVT4vy6+nXnrRl7RNOYu9GkTUowFyIxmBbSEQh3vMa7HmaWmPblJmhJ52lLRKD67Ph5vXDJ8qx74M9E6T8WT9FsD6kZATyAdB04t89/1O/w1cDnyilFU=";
String groupId = "C050a77552ee3823d67595ac3911107e5";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("WiFi Connected!");
}

void sendLineMessage(String message) {
  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return;
  }

  String url = "/v2/bot/message/push";
  
  String payload = "{\"to\":\"" + groupId + "\",\"messages\":[{\"type\":\"text\",\"text\":\"" + message + "\"}]}";
  
  String request = "POST " + url + " HTTP/1.1\r\n" +
                   "Host: " + String(host) + "\r\n" +
                   "Authorization: Bearer " + accessToken + "\r\n" +
                   "Content-Type: application/json\r\n" +
                   "Content-Length: " + String(payload.length()) + "\r\n\r\n" +
                   payload;
  
  client.print(request);
  Serial.println("Message Sent!");
}

void loop() {
  sendLineMessage("📢 แจ้งเตือน: Hello from ESP8266 to Group!");
  delay(60000);
}
