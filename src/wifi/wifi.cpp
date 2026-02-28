#include "wifi.h"

#include <Arduino.h>
#include "../debug.h"
#include <WiFi.h>
#include <Preferences.h>

static const char* PREFS_NAMESPACE  = "wifi_prefs";
static const char* PREFS_KEY_SSID   = "wifi_ssid";
static const char* PREFS_KEY_PASS   = "wifi_password";
const int BOOT_BUTTON_PIN           = 0;
const int WIFI_CONNECT_BLINK_DELAY  = 500;

static bool _stopConnectingToWifi = false;

void connectToWifi() {
  _stopConnectingToWifi = false;

  Preferences prefs;
  prefs.begin(PREFS_NAMESPACE);

  if (!prefs.isKey(PREFS_KEY_SSID) && !prefs.isKey(PREFS_KEY_PASS)) {
    LOG("[Wi-Fi] No Wi-Fi credentials found");
    prefs.end();
    return;
  }
  
  String ssid = prefs.getString(PREFS_KEY_SSID, "");
  String password = prefs.getString(PREFS_KEY_PASS, "");
  
  prefs.end();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  LOG("[Wi-Fi] Connecting to " + ssid);

  toggleGreenPin(false);

  while (WiFi.status() != WL_CONNECTED) {
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) return;

    if (_stopConnectingToWifi) {
        WiFi.disconnect();
        _stopConnectingToWifi = false;
        return;
    }

    toggleYellowPin(true);
    delay(WIFI_CONNECT_BLINK_DELAY);

    toggleYellowPin(false);
    delay(WIFI_CONNECT_BLINK_DELAY);
  }

  toggleGreenPin(true);

  LOG("[Wi-Fi] Connected to " + ssid);
}

void writeWifiConf(String ssid, String password) {
  Preferences prefs;
  prefs.begin("wifi_prefs");

  prefs.putString(PREFS_KEY_SSID, ssid);
  prefs.putString(PREFS_KEY_PASS, password);

  prefs.end();
}

void stopConnectingToWifi() {
    _stopConnectingToWifi = true;
}

bool isConnectedToWifi() {
  return WiFi.status() == WL_CONNECTED;
}