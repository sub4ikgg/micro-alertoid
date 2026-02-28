#include <Arduino.h>
#include <HTTPClient.h>
#include "led/led.h"
#include "wifi/wifi.h"
#include "ble/ble.h"
#include "debug.h"

WiFiClientSecure secureClient;
WiFiClient client;

const int BOOT_BUTTON_PIN = 0;
const int SERIAL_BAUD_RATE = 115200;
const int WIFI_CLIENT_TIMEOUT = 5;
const int BLE_TOGGLE_PRESS_THRESHOLD = 3;
const int BLE_AUTO_STOP_INTERVAL = 60;
const int RESOURCE_CHECK_INTERVAL = 100;
const int HTTP_OK = 200;

int bleToggleCounter = 0;
int bleAutoStopCounter = 0;
int resourceCheckingCounter = RESOURCE_CHECK_INTERVAL;

void setInsecureWifiClient();
void checkResourceAvailability();

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  client.setTimeout(WIFI_CLIENT_TIMEOUT);

  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  preparePinMode();
  testLeds();

  initBle();
  setInsecureWifiClient();
  connectToWifi();
}

void setInsecureWifiClient() {
  secureClient.setInsecure();
}

void loop() {
  if (digitalRead(BOOT_BUTTON_PIN) == LOW && !isBleAdvertising) {
    if (++bleToggleCounter > BLE_TOGGLE_PRESS_THRESHOLD)
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

    if (++bleAutoStopCounter >= BLE_AUTO_STOP_INTERVAL) {
      bleAutoStopCounter = 0;
      stopBleAdvertising();
    }

    return;
  }
  
  if (!isConnectedToWifi()) {
    LOG("[Main] Device is not connected to Wi-Fi. Trying to connect...");

    waitingForWifiBlink();
    connectToWifi();
    return;
  }

  if (resourceCheckingCounter >= RESOURCE_CHECK_INTERVAL) {
    resourceCheckingCounter = 0;
    checkResourceAvailability();
  } else {
    resourceCheckingCounter++;
    delay(100);
  }
}

void checkResourceAvailability() {
  if (digitalRead(BOOT_BUTTON_PIN) == LOW && isBleAdvertising) return;

  LOG("[Main] Checking resource availability...");
  
  HTTPClient http;
  http.begin("https://httpbin.org/status/200");

  int code = http.GET();
  http.end();

  if (code < 0) {
    LOG("[Main] Retrying...");
    return;
  }

  if (code == HTTP_OK) {
    toggleGreenPin(true);
    toggleRedPin(false);

    LOG("[Main] Resource is available (200 OK)");
  } else {
    LOG("[Main] Resource is not available (" + String(code) + ")");
    resourceIsNotAvailableBlink();
  }
}