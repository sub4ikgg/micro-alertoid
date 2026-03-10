#ifndef LED_H
#define LED_H

void preparePinMode();
void testLeds();
void waitingForWifiBlink();
void resourceIsNotAvailableBlink();
void bleIsEnabledBlink();
void bleDeviceIsConnectedBlink();

void toggleYellowPin(bool val);
void toggleGreenPin(bool val);
void toggleRedPin(bool val);

#endif