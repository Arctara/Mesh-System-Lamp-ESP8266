/*
    PINOUT
      PIN 12 = Zero Crossing
      PIN 14 = PWM Output
*/

//$ Include Library
#include <Arduino.h>
#include <ArduinoJson.h>
#include <RBDdimmer.h>
#include <WebSocketsClient.h>
// #include <painlessMesh.h>

//$ Mesh Configuration
// #define MESH_PREFIX "ALiVe_MESH"
// #define MESH_PASSWORD "TmlhdCBzZWthbGkgYW5kYSBtZW5kZWNvZGUgaW5pIC1NZXJ6YQ=="
// #define MESH_PORT 5555

//$ Access Point Configuration
#define WIFI_SSID "ALiVe_AP"
#define WIFI_PASS "LeTS_ALiVe"

#define outputPin 14
#define zerocross 12

//*Mesh Configuration
// Scheduler userScheduler;
// painlessMesh mesh;
// int nodeNumber = 1;

bool lampCondition = false;

WebSocketsClient webSocket;

DynamicJsonDocument data(1024);
DynamicJsonDocument receivedData(1024);
dimmerLamp dimmer(outputPin, zerocross);

void sendMessage();
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
// Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

//$ Needed for painless mesh library
// void receivedCallback(uint32_t from, String &msg);
// void newConnectionCallback(uint32_t nodeId);
// void changedConnectionCallback();
// void nodeTimeAdjustedCallback(int32_t offset);

void setup() {
  Serial.begin(115200);
  // mesh.setDebugMsgTypes(ERROR | STARTUP);

  dimmer.begin(NORMAL_MODE, ON);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  webSocket.begin("192.168.5.1", 80, "/ws");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);

  // mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  // mesh.onReceive(&receivedCallback);
  // mesh.onNewConnection(&newConnectionCallback);
  // mesh.onChangedConnections(&changedConnectionCallback);
  // mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  // userScheduler.addTask(taskSendMessage);
  // taskSendMessage.enable();
}

void loop() {
  // mesh.update();
  webSocket.loop();
}

void sendMessage() {
  data["from"] = "lamp-1";
  data["to"] = "center";
  data["feedback"] = lampCondition;
  String msg;
  serializeJson(data, msg);
  webSocket.sendTXT(msg);
  // mesh.sendBroadcast(msg);
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
    sendMessage();
  }
}

//$ Needed for painless mesh library
// void receivedCallback(uint32_t from, String &msg) {
//   Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
//   deserializeJson(receivedData, msg);
//   if (data["from"].as<String>() == "center" &&
//       data["to"].as<String>() == "lamp-1") {
//     if (data["condition"].as<String>() == "true") {
//       plugCondition = true;
//       dimmer.setState(ON);
//     } else {
//       plugCondition = false;
//       dimmer.setState(OFF);
//     }
//   }
//   sendMessage();
// }

// void newConnectionCallback(uint32_t nodeId) {
//   Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
// }

// void changedConnectionCallback() { Serial.printf("Changed connections\n"); }

// void nodeTimeAdjustedCallback(int32_t offset) {
//   Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),
//   offset);
// }