int device1 = 5;
int device2 = 4;
int device3 = 0;
int device4 = 2;
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> // get it from https://github.com/Links2004/arduinoWebSockets/releases 
#include <ArduinoJson.h> // get it from https://arduinojson.org/ or install via Arduino library manager
#include <StreamString.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;

#define MyApiKey "XXXXXXXXXXXXx" // TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define MySSID "XXXXXXXXXXXXX" // TODO: Change to your Wifi network SSID
#define MyWifiPassword "MyTopSecretPassword" // TODO: Change to your Wifi network password

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes

uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

void setPowerStateOnServer(String deviceId, String value);
void setTargetTemperatureOnServer(String deviceId, String value, String scale);

// deviceId is the ID assgined to your smart-home-device in sinric.com dashboard. Copy it from dashboard and paste it here

void turnOn(String deviceId) {
 if (deviceId == "5bed79eb4c865b7b042fad1a") // Device ID of device 1
 { 
 Serial.print("Turn on device id: ");
 Serial.println(deviceId);
 digitalWrite(device1, LOW);
 } 
 else if (deviceId == "5bed7a064c865b7b042fad1c") // Device ID of device 2
 { 
 Serial.print("Turn on device id: ");
 Serial.println(deviceId);
 digitalWrite(device2, LOW);
 }
 else if (deviceId == "5bed7a114c865b7b042fad1d") // Device ID of device 3
 { 
 Serial.print("Turn on device id: ");
 Serial.println(deviceId);
 digitalWrite(device3, LOW);
 } 
 else if (deviceId == "5bed7a1d4c865b7b042fad1e") // Device ID of device 4
 { 
 Serial.print("Turn on device id: ");
 Serial.println(deviceId);
 digitalWrite(device4, LOW);
 }
 else 
 {
 Serial.print("Turn on for unknown device id: ");
 Serial.println(deviceId); 
 } 
}

void turnOff(String deviceId) {
 if (deviceId == "5bed79eb4c865b7b042fad1a") // Device ID of device 1
 { 
 Serial.print("Turn off Device ID: ");
 Serial.println(deviceId);
 digitalWrite(device1, HIGH);
 }
 else if (deviceId == "5bed7a064c865b7b042fad1c") // Device ID of device 2
 { 
 Serial.print("Turn off Device ID: ");
 Serial.println(deviceId);
 digitalWrite(device2, HIGH);
 }
 else if (deviceId == "5bed7a114c865b7b042fad1d") // Device ID of device 3
 { 
 Serial.print("Turn off Device ID: ");
 Serial.println(deviceId);
 digitalWrite(device3, HIGH);
 }
 else if (deviceId == "5bed7a1d4c865b7b042fad1e") // Device ID of device 4
 { 
 Serial.print("Turn off Device ID: ");
 Serial.println(deviceId);
 digitalWrite(device4, HIGH);
 }
 else 
 {
 Serial.print("Turn off for unknown device id: ");
 Serial.println(deviceId); 
 }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
 switch(type) {
 case WStype_DISCONNECTED:
 isConnected = false; 
 Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
 break;
 case WStype_CONNECTED: {
 isConnected = true;
 Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
 Serial.printf("Waiting for commands from sinric.com ...\n"); 
 }
 break;
 case WStype_TEXT: {
 Serial.printf("[WSc] get text: %s\n", payload);
 // Example payloads

// For Switch or Light device types
 // {"deviceId": xxxx, "action": "setPowerState", value: "ON"} // https://developer.amazon.com/docs/device-apis/alexa-powercontroller.html

// For Light device type
 // Look at the light example in github
 
 DynamicJsonBuffer jsonBuffer;
 JsonObject& json = jsonBuffer.parseObject((char*)payload); 
 String deviceId = json ["deviceId"]; 
 String action = json ["action"];
 
 if(action == "setPowerState") { // Switch or Light
 String value = json ["value"];
 if(value == "ON") {
 turnOn(deviceId);
 } else {
 turnOff(deviceId);
 }
 }
 else if (action == "SetTargetTemperature") {
 String deviceId = json ["deviceId"]; 
 String action = json ["action"];
 String value = json ["value"];
 }
 else if (action == "test") {
 Serial.println("[WSc] received test command from sinric.com");
 }
 }
 break;
 case WStype_BIN:
 Serial.printf("[WSc] get binary length: %u\n", length);
 break;
 }
}

void setup() 
{
 pinMode (device1, OUTPUT);
 pinMode (device2, OUTPUT);
 pinMode (device3, OUTPUT);
 pinMode (device4, OUTPUT);




Serial.begin(115200);
 
 WiFiMulti.addAP(MySSID, MyWifiPassword);
 Serial.println();
 Serial.print("Connecting to Wifi: ");
 Serial.println(MySSID);

// Waiting for Wifi connect
 while(WiFiMulti.run() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }
 if(WiFiMulti.run() == WL_CONNECTED) {
 Serial.println("");
 Serial.print("WiFi connected. ");
 Serial.print("IP address: ");
 Serial.println(WiFi.localIP());
 }

// server address, port and URL
 webSocket.begin("iot.sinric.com", 80, "/");

// event handler
 webSocket.onEvent(webSocketEvent);
 webSocket.setAuthorization("apikey", MyApiKey);
 
 // try again every 5000ms if connection has failed
 webSocket.setReconnectInterval(5000); // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets
}

void loop() {
 
 webSocket.loop();
 
 if(isConnected) {
 uint64_t now = millis();
 
 // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
 if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
 heartbeatTimestamp = now;
 webSocket.sendTXT("H"); 
 }
 } 
}

// If you are going to use a push button to on/off the switch manually, use this function to update the status on the server
// so it will reflect on Alexa app.
// eg: setPowerStateOnServer("deviceid", "ON")
void setPowerStateOnServer(String deviceId, String value) {
 DynamicJsonBuffer jsonBuffer;
 JsonObject& root = jsonBuffer.createObject();
 root["deviceId"] = deviceId;
 root["action"] = "setPowerState";
 root["value"] = value;
 StreamString databuf;
 root.printTo(databuf);
 
 webSocket.sendTXT(databuf);
}

//eg: setPowerStateOnServer("deviceid", "CELSIUS", "25.0")
void setTargetTemperatureOnServer(String deviceId, String value, String scale) {
 DynamicJsonBuffer jsonBuffer;
 JsonObject& root = jsonBuffer.createObject();
 root["action"] = "SetTargetTemperature";
 root["deviceId"] = deviceId;
 
 JsonObject& valueObj = root.createNestedObject("value");
 JsonObject& targetSetpoint = valueObj.createNestedObject("targetSetpoint");
 targetSetpoint["value"] = value;
 targetSetpoint["scale"] = scale;
 
 StreamString databuf;
 root.printTo(databuf);
 
 webSocket.sendTXT(databuf);
}
