#ifndef WIFI_H
#define WIFI_H

#include "led/led.h"
#include <Arduino.h>

void connectToWifi();
void writeWifiConf(String ssid, String passphrase);
void stopConnectingToWifi();
bool isConnectedToWifi();

#endif