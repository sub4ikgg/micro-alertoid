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
static const char* PREFS_KEY_INTERVAL    = "res_interval";
static const char* DEFAULT_RESOURCE_URL  = "https://httpbin.org/status/200";
const int DEFAULT_EXPECTED_CODE          = 200;
const int DEFAULT_CHECK_INTERVAL         = 50;

static String _url;
static int _expectedCode    = -1;
static int _checkInterval   = -1;

static void loadPrefsIfNeeded();

void checkResourceAvailability() {
  LOG("[Resource] Left memory before check: " + String(ESP.getMaxAllocHeap()));

  if (digitalRead(BOOT_BUTTON_PIN) == LOW && isBleAdvertising) return;

  loadPrefsIfNeeded();

  LOG(F("[Resource] Checking resource availability..."));
  LOG("[Resource] " + _url);

  HTTPClient http;
  http.begin(_url.c_str());

  int code = http.GET();
  http.end();

  if (code < 0) {
    LOG(F("[Resource] Retrying..."));
    LOG("[Resource] Left memory after failure: " + String(ESP.getMaxAllocHeap()));
    return;
  }

  if (code == _expectedCode) {
    toggleRedPin(false);

    toggleGreenPin(true);
    delay(100);
    toggleGreenPin(false);
    delay(100);
    toggleGreenPin(true);
    delay(100);
    toggleGreenPin(false);
    delay(100);
    toggleGreenPin(true);

    LOG("[Resource] Available (" + String(_expectedCode) + ")");
  } else {
    LOG("[Resource] Not available (" + String(code) + ")");
    resourceIsNotAvailableBlink();
  }

  LOG("[Resource] Left memory after check: " + String(ESP.getMaxAllocHeap()));
}

static void loadPrefsIfNeeded() {
  if (_expectedCode != -1) return;

  Preferences prefs;
  prefs.begin(PREFS_NAMESPACE);
  _url           = prefs.isKey(PREFS_KEY_URL)      ? prefs.getString(PREFS_KEY_URL)  : DEFAULT_RESOURCE_URL;
  _expectedCode  = prefs.isKey(PREFS_KEY_CODE)     ? prefs.getInt(PREFS_KEY_CODE)    : DEFAULT_EXPECTED_CODE;
  _checkInterval = prefs.isKey(PREFS_KEY_INTERVAL) ? prefs.getInt(PREFS_KEY_INTERVAL): DEFAULT_CHECK_INTERVAL;
  prefs.end();
}

void writeResourceConf(String url, int expectedCode, int checkInterval) {
  Preferences prefs;
  prefs.begin(PREFS_NAMESPACE);
  prefs.putString(PREFS_KEY_URL, url);
  prefs.putInt(PREFS_KEY_CODE, expectedCode);
  prefs.putInt(PREFS_KEY_INTERVAL, checkInterval);
  prefs.end();

  _url           = url;
  _expectedCode  = expectedCode;
  _checkInterval = checkInterval;
}

int getResourceCheckInterval() {
  loadPrefsIfNeeded();
  return _checkInterval;
}

String getResourceUrl() {
  loadPrefsIfNeeded();
  return _url;
}

int getResourceExpectedCode() {
  loadPrefsIfNeeded();
  return _expectedCode;
}
