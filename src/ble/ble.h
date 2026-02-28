#ifndef BLE_H
#define BLE_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define BLE_SERVICE_UUID   "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_TX_UUID        "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_WIFI_CONF_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_URL_CONF_UUID  "6E400004-B5A3-F393-E0A9-E50E24DCCA9E"

extern bool isBleDeviceConnected;
extern bool isBleAdvertising;

void initBle();
void startBleAdvertising();
void stopBleAdvertising();

#endif