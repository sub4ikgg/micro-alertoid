#include <Arduino.h>
#include <HTTPClient.h>
#include "led/led.h"
#include "wifi/wifi.h"
#include "ble/ble.h"

WiFiClientSecure secureClient;
WiFiClient client;

int bleToggleCounter = 0;

void setInsecureWifiClient();
void checkResourceAvailability();

void setup() {
  Serial.begin(115200);
  client.setTimeout(5);

  pinMode(0, INPUT_PULLUP);
  preparePinMode();
  testLeds();

  setInsecureWifiClient();
  connectToWifi();
  initBle();
}

void setInsecureWifiClient() {
  secureClient.setInsecure();
}

void loop() {
  if (digitalRead(0) == LOW && !isBleAdvertising) {
    bleToggleCounter++;

    if (bleToggleCounter > 3)
    {
      bleToggleCounter = 0;
      startBleAdvertising();
    }

    delay(1000);
    return;
  } else if (bleToggleCounter > 0) {
    bleToggleCounter = 0;
  }

  if (isBleAdvertising) {
    bleIsEnabledBlink();
    return;
  }
  
  if (!isConnectedToWifi()) {
    Serial.println("[Main] Device is not connected to Wi-Fi. Trying to connect...");

    waitingForWifiBlink();
    connectToWifi();
    return;
  }

  checkResourceAvailability();
}

void checkResourceAvailability() {
  Serial.println("Free Heap: " + String(ESP.getFreeHeap()));
  Serial.print("[Main] Checking resource availability... ");
  
  HTTPClient http;
  http.begin("https://httpbin.org/status/503");

  int code = http.GET();
  http.end();

  if (code < 0) {
    Serial.println("Retrying..."); 
    return;
  }

  if (code == 200) {
    toggleGreenPin(true);
    toggleRedPin(false);

    Serial.println("Resource is available (200 OK)");
    delay(10000);
  } else {
    Serial.println("Resource is not available (" + String(code) + ")");
    resourceIsNotAvailableBlink();
  }
}