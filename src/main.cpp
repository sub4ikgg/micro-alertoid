#include <Arduino.h>
#include <HTTPClient.h>
#include "led/led.h"
#include "wifi/wifi.h"
#include "ble/ble.h"

void checkResourceAvailability();

void setup() {
  Serial.begin(115200);

  preparePinMode();
  testLeds();

  initBle();
  connectToWifi();
}

void loop() {
  while (!isConnectedToWifi()) {
    Serial.println("[Main] Device is not connected to Wi-Fi. Trying to connect...");

    waitingForWifiBlink();
    connectToWifi();
  }

  checkResourceAvailability();
  
  delay(10000);
}

void checkResourceAvailability() {
  Serial.print("[Main] Checking resource availability... ");
  
  HTTPClient http;
  http.begin("http://jsonplaceholder.typicode.com/todos/1");

  int code = http.GET();
  if (code < 0) {
    Serial.println("Retrying...");

    http.end();    
    return;
  }

  if (code == 200) {
    toggleGreenPin(true);
    toggleRedPin(false);

    Serial.println("Resource is available (200 OK)");
  } else {
    toggleRedPin(true);
    toggleGreenPin(false);

    Serial.println("Resource is not available (" + String(code) + ")");
  }

  http.end();
}