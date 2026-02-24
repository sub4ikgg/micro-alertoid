#ifndef WIFI_H
#define WIFI_H

#include "led/led.h"
#include <Arduino.h>

void connectToWifi();
void writeWifiConf(String ssid, String password);
void stopConnectingToWifi();
bool isConnectedToWifi();
static String getDeviceName();

#endif