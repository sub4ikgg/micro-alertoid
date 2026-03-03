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
static String _ssid;
static String _password;
static bool _prefsLoaded = false;

static void loadPrefsIfNeeded();

void connectToWifi() {
  _stopConnectingToWifi = false;

  loadPrefsIfNeeded();

  if (_ssid.isEmpty()) return;

  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid, _password);
  LOG("[Wi-Fi] Connecting to " + _ssid);

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

  LOG("[Wi-Fi] Connected to " + _ssid);
}

static void loadPrefsIfNeeded() {
  if (_prefsLoaded) return;

  Preferences prefs;
  prefs.begin(PREFS_NAMESPACE);

  if (!prefs.isKey(PREFS_KEY_SSID) && !prefs.isKey(PREFS_KEY_PASS)) {
    LOG(F("[Wi-Fi] No Wi-Fi credentials found"));
    prefs.end();
    _prefsLoaded = true;
    return;
  }

  _ssid     = prefs.getString(PREFS_KEY_SSID, "");
  _password = prefs.getString(PREFS_KEY_PASS, "");
  prefs.end();
  _prefsLoaded = true;
}

void writeWifiConf(String ssid, String password) {
  Preferences prefs;
  prefs.begin(PREFS_NAMESPACE);
  prefs.putString(PREFS_KEY_SSID, ssid);
  prefs.putString(PREFS_KEY_PASS, password);
  prefs.end();

  _ssid        = ssid;
  _password    = password;
  _prefsLoaded = true;
}

void stopConnectingToWifi() {
    _stopConnectingToWifi = true;
}

bool isConnectedToWifi() {
  return WiFi.status() == WL_CONNECTED;
}