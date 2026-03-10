#include "led.h"

#include <Arduino.h>

const uint8_t Y_PIN = 27;
const uint8_t G_PIN = 25;
const uint8_t R_PIN = 32;

void preparePinMode() {
  pinMode(Y_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(R_PIN, OUTPUT);
}

void testLeds() {
  for(int i = 0; i < 2; i++) {
    toggleYellowPin(true);
    toggleGreenPin(true);
    toggleRedPin(true);

    delay(100);

    toggleYellowPin(false);
    toggleGreenPin(false);
    toggleRedPin(false);

    delay(100);
  }
}

void waitingForWifiBlink() {
  toggleRedPin(false);

  toggleYellowPin(true);
  toggleGreenPin(true);

  delay(500);

  toggleYellowPin(false);
  toggleGreenPin(false);

  delay(500);
}

static void waitingForBleBlink() {
  toggleYellowPin(false);
  toggleGreenPin(false);
  toggleRedPin(false);

  toggleYellowPin(true);

  delay(300);

  toggleYellowPin(false);

  toggleGreenPin(true);

  delay(300);

  toggleGreenPin(false);

  toggleRedPin(true);

  delay(300);

  toggleRedPin(false);
}

void resourceIsNotAvailableBlink() {
  toggleYellowPin(false);
  toggleGreenPin(false);
  toggleRedPin(false);

  for (size_t i = 0; i < 10; i++)
  {
    toggleRedPin(true);

    delay(50);

    toggleRedPin(false);

    delay(50);
  }

  toggleRedPin(true);
}

void bleDeviceIsConnectedBlink() {
  toggleYellowPin(false);
  toggleGreenPin(false);
  toggleRedPin(false);

  toggleYellowPin(true);

  delay(250);

  toggleRedPin(true);

  delay(250);

  toggleYellowPin(false);

  delay(250);

  toggleRedPin(false);

  delay(250);
}

void bleIsEnabledBlink() {
  toggleYellowPin(false);
  toggleGreenPin(false);
  toggleRedPin(false);

  toggleYellowPin(true);
  toggleRedPin(true);

  delay(250);

  toggleYellowPin(false);
  toggleRedPin(false);

  delay(250);
}

void toggleYellowPin(bool val) {
  digitalWrite(Y_PIN, val ? HIGH : LOW);
}

void toggleGreenPin(bool val) {
  digitalWrite(G_PIN, val ? HIGH : LOW);
}

void toggleRedPin(bool val) {
  digitalWrite(R_PIN, val ? HIGH : LOW);
}