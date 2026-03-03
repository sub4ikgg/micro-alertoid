#include "resource.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include "../debug.h"
#include "../led/led.h"
#include "../ble/ble.h"

const int BOOT_BUTTON_PIN = 0;

static const char* PREFS_NAMESPACE       = "res_prefs";
static const char* PREFS_KEY_URL         = "res_url";
static const char* PREFS_KEY_CODE        = "res_code";
static const char* DEFAULT_RESOURCE_URL  = "https://httpbin.org/status/200";
const int DEFAULT_EXPECTED_CODE          = 200;

static String _url;
static int _expectedCode = -1;

static void loadPrefsIfNeeded();

void checkResourceAvailability() {
  if (digitalRead(BOOT_BUTTON_PIN) == LOW && isBleAdvertising) return;

  loadPrefsIfNeeded();

  LOG(F("[Resource] Checking resource availability..."));

  HTTPClient http;
  http.begin(_url);

  int code = http.GET();
  http.end();

  if (code < 0) {
    LOG(F("[Resource] Retrying..."));
    return;
  }

  if (code == _expectedCode) {
    toggleRedPin(false);

    toggleGreenPin(true);
    delay(50);
    toggleGreenPin(false);
    delay(50);
    toggleGreenPin(true);

    LOG("[Resource] Available (" + String(_expectedCode) + ")");
  } else {
    LOG("[Resource] Not available (" + String(code) + ")");
    resourceIsNotAvailableBlink();
  }
}

static void loadPrefsIfNeeded() {
  if (_expectedCode != -1) return;

  Preferences prefs;
  prefs.begin(PREFS_NAMESPACE, true);
  _url          = prefs.getString(PREFS_KEY_URL, DEFAULT_RESOURCE_URL);
  _expectedCode = prefs.getInt(PREFS_KEY_CODE, DEFAULT_EXPECTED_CODE);
  prefs.end();
}

void writeResourceConf(String url, int expectedCode) {
  Preferences prefs;
  prefs.begin(PREFS_NAMESPACE);
  prefs.putString(PREFS_KEY_URL, url);
  prefs.putInt(PREFS_KEY_CODE, expectedCode);
  prefs.end();

  _url = url;
  _expectedCode = expectedCode;
}
