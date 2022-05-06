//$ Include Library
#include <Arduino.h>
#include <ArduinoJson.h>
#include <RBDdimmer.h>
#include <WebSocketsClient.h>

//$ Access Point Configuration
#define WIFI_SSID "ALiVe_AP"
#define WIFI_PASS "LeTS_ALiVe"

#define outputPin 14
#define zerocross 12
#define feedbackPin A0

//* Device Name
const String deviceName = "lamp-1";
const String centerName = "center";

unsigned long prevMillis = 0;
int sensorRead = 0;
String feedback;
String prevFeedback = "OFF";

bool lampCondition = false;

WebSocketsClient webSocket;

DynamicJsonDocument data(1024);
DynamicJsonDocument receivedData(1024);
dimmerLamp dimmer(outputPin, zerocross);

void sendMessage();
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);

void setup() {
  Serial.begin(115200);
  pinMode(feedbackPin, INPUT);

  dimmer.begin(NORMAL_MODE, ON);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  webSocket.begin("192.168.5.1", 80, "/ws");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();

  sensorRead = analogRead(feedbackPin);

  if (sensorRead < 200) {
    feedback = "ON";
  } else if (sensorRead >= 200) {
    feedback = "OFF";
  }

  if (feedback != prevFeedback) {
    sendMessage();
  }

  prevFeedback = feedback;
}

void sendMessage() {
  data["from"] = deviceName;
  data["to"] = centerName;
  data["feedback"] = feedback;
  String msg;
  serializeJson(data, msg);
  webSocket.sendTXT(msg);
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_TEXT) {
    deserializeJson(receivedData, payload);

    String myData;
    serializeJson(receivedData, myData);

    String from = receivedData["from"].as<String>();
    String to = receivedData["to"].as<String>();
    String condition = receivedData["condition"].as<String>();

    Serial.println(myData);
    Serial.println(from);
    Serial.println(to);
    Serial.println(condition);

    if (to == "lamp-1") {
      Serial.println("Data is for this device!");
      if (from == "center") {
        Serial.println("Data from center!");
        if (condition == "1") {
          lampCondition = true;
          dimmer.setState(ON);
          Serial.println("True!");
        } else {
          lampCondition = false;
          dimmer.setState(OFF);
          Serial.println("False!");
        }
      }
    }
  }
}