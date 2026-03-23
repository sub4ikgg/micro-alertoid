#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
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

int bleToggleCounter = 0;
int bleAutoStopCounter = 0;
int resourceCheckingCounter = 0;

static void setInsecureWifiClient();
static void handleNewWifiConfig();
static void handleNewUrlConfig();

void setup() {
  if (DEBUG) {
    Serial.begin(SERIAL_BAUD_RATE);
  }
  
  client.setTimeout(WIFI_CLIENT_TIMEOUT);

  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  preparePinMode();
  testLeds();

  setInsecureWifiClient();

  if (esp_reset_reason() == ESP_RST_SW) {
    LOG(F("[BLE] Auto-advertising after SW reboot"));
    startBleAdvertising();
  } else {
    connectToWifi();
  }

  resourceCheckingCounter = getResourceCheckInterval();

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) { delay(500); }
}

static void setInsecureWifiClient() {
  secureClient.setInsecure();
}

static void handleNewWifiConfig() {
  if (!isWifiConfigChanged) return;
  isWifiConfigChanged = false;

  JsonDocument doc;
  if (deserializeJson(doc, newWifiConfig) != DeserializationError::Ok) {
    LOG(F("[Loop] New wifi config: invalid JSON"));
    return;
  }

  const char* ssid       = doc["ssid"];
  const char* passphrase = doc["passphrase"];

  if (!ssid || !passphrase) {
    LOG(F("[Loop] New wifi config: missing ssid or passphrase"));
    return;
  }

  LOG("[Loop] Applying new Wi-Fi config: " + String(ssid));
  writeWifiConf(ssid, passphrase);
  connectToWifi();
}

static void handleNewUrlConfig() {
  if (!isUrlConfigChanged) return;
  isUrlConfigChanged = false;

  JsonDocument doc;
  if (deserializeJson(doc, newUrlConfig) != DeserializationError::Ok) {
    LOG(F("[Loop] New url config: invalid JSON"));
    return;
  }

  const char* url = doc["url"];
  int code        = doc["code"].as<int>();
  int interval    = doc["check_interval"].as<int>() * 10;

  if (!url) {
    LOG(F("[Loop] New url config: missing url"));
    return;
  }

  LOG(F("[Loop] Applying new url config"));
  writeResourceConf(url, code, interval);
}

void loop() {
  if (digitalRead(BOOT_BUTTON_PIN) == LOW && !isBleAdvertising) {
    toggleRedPin(false);
    toggleGreenPin(false);
    toggleYellowPin(true);

    if (++bleToggleCounter > BLE_TOGGLE_PRESS_THRESHOLD)
    {
      bleToggleCounter = 0;
      initBle();
      startBleAdvertising();
    }

    delay(500);
    return;
  } else if (bleToggleCounter > 0) {
    bleToggleCounter = 0;
  }

  handleNewWifiConfig();
  handleNewUrlConfig();

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

        resourceCheckingCounter = getResourceCheckInterval();
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

  if (resourceCheckingCounter >= getResourceCheckInterval()) {
    resourceCheckingCounter = 0;
    checkResourceAvailability();
  } else {
    resourceCheckingCounter++;
    delay(100);
  }
}
