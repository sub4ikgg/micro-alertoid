#include "resource.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include "../debug.h"
#include "../led/led.h"
#include "../ble/ble.h"

const int HTTP_OK = 200;
const int BOOT_BUTTON_PIN = 0;

void checkResourceAvailability() {
  if (digitalRead(BOOT_BUTTON_PIN) == LOW && isBleAdvertising) return;

  LOG("[Resource] Checking resource availability...");

  HTTPClient http;
  http.begin("https://httpbin.org/status/200");

  int code = http.GET();
  http.end();

  if (code < 0) {
    LOG("[Resource] Retrying...");
    return;
  }

  if (code == HTTP_OK) {
    toggleRedPin(false);

    toggleGreenPin(true);
    delay(50);
    toggleGreenPin(false);
    delay(50);
    toggleGreenPin(true);

    LOG("[Resource] Available (200 OK)");
  } else {
    LOG("[Resource] Not available (" + String(code) + ")");
    resourceIsNotAvailableBlink();
  }
}
