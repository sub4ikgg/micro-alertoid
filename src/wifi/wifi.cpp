#include "wifi.h"

#include <Arduino.h>
#include "../debug.h"
#include <WiFi.h>
#include <Preferences.h>

static bool _stopConnectingToWifi = false;

void connectToWifi() {
  _stopConnectingToWifi = false;

  Preferences prefs;
  prefs.begin("wifi_prefs");

  if (!prefs.isKey("wifi_ssid") && !prefs.isKey("wifi_password")) {
    LOG("[Wi-Fi] No Wi-Fi credentials found");
    prefs.end();
    return;
  }
  
  String ssid = prefs.getString("wifi_ssid", "");
  String password = prefs.getString("wifi_password", "");
  
  prefs.end();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  LOG("[Wi-Fi] Connecting to " + ssid);

  toggleGreenPin(false);

  while (WiFi.status() != WL_CONNECTED) {
    if (digitalRead(0) == LOW) return;

    if (_stopConnectingToWifi) {
        WiFi.disconnect();
        _stopConnectingToWifi = false;
        return;
    }

    toggleYellowPin(true);
    delay(500);

    toggleYellowPin(false);
    delay(500);
  }

  toggleGreenPin(true);

  LOG("[Wi-Fi] Connected to " + ssid);
}

void writeWifiConf(String ssid, String password) {
  Preferences prefs;
  prefs.begin("wifi_prefs");

  prefs.putString("wifi_ssid", ssid);
  prefs.putString("wifi_password", password);

  prefs.end();
}

void stopConnectingToWifi() {
    _stopConnectingToWifi = true;
}

bool isConnectedToWifi() {
  return WiFi.status() == WL_CONNECTED;
}