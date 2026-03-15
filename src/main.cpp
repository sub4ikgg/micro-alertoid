#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "led/led.h"
#include "wifi/wifi.h"
#include "ble/ble.h"
#include "resource/resource.h"
#include "debug.h"

WiFiClientSecure secureClient;
WiFiClient client;

const int BOOT_BUTTON_PIN            = 0;
const int SERIAL_BAUD_RATE           = 115200;
const int WIFI_CLIENT_TIMEOUT        = 5;
const int BLE_TOGGLE_PRESS_THRESHOLD = 3;
const int BLE_AUTO_STOP_INTERVAL     = 240; // 120 sec.
const int RESOURCE_CHECK_INTERVAL    = 30; // 5 sec.

int bleToggleCounter = 0;
int bleAutoStopCounter = 0;
int resourceCheckingCounter = RESOURCE_CHECK_INTERVAL;

static void setInsecureWifiClient();

void setup() {
  if (DEBUG) {
    Serial.begin(SERIAL_BAUD_RATE);
  }
  
  client.setTimeout(WIFI_CLIENT_TIMEOUT);

  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  preparePinMode();
  testLeds();

  initBle();

  if (esp_reset_reason() == ESP_RST_SW) {
    LOG(F("[BLE] Auto-advertising after SW reboot"));
    startBleAdvertising();
  }

  setInsecureWifiClient();
  connectToWifi();
}

static void setInsecureWifiClient() {
  // secureClient.setInsecure();
}

void loop() {
  if (digitalRead(BOOT_BUTTON_PIN) == LOW && !isBleAdvertising) {
    toggleRedPin(false);
    toggleGreenPin(false);
    toggleYellowPin(true);

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

  if (isBleAdvertising || isBleDeviceConnected) {
    if (isBleDeviceConnected) {
      bleDeviceIsConnectedBlink();
    } else {
      bleIsEnabledBlink();
    }

    if (!isBleDeviceConnected) {
      if (++bleAutoStopCounter >= BLE_AUTO_STOP_INTERVAL) {
        bleAutoStopCounter = 0;
        stopBleAdvertising();

        resourceCheckingCounter = RESOURCE_CHECK_INTERVAL;
      }
    }

    return;
  }
  
  if (!isConnectedToWifi()) {
    LOG(F("[Loop] Device is not connected to Wi-Fi. Trying to connect..."));

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
