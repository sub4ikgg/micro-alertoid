#include "ble.h"
#include <Arduino.h>
#include "wifi/wifi.h"
#include "../debug.h"

bool isBleInitialized = false;
bool isBleDeviceConnected = false;
bool isBleAdvertising = false;

static BLECharacteristic *pTxChar;

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *s) {
        isBleDeviceConnected = true;
        LOG("[BLE] Device connected");
    }

    void onDisconnect(BLEServer *s) {
        isBleDeviceConnected = false;
        LOG("[BLE] Device disconnected");
    }
};

class WifiConfCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
        // conf format: "ssid;password"
        String msg = pChar->getValue().c_str();

        if (msg.length() > 0) {
            int separatorIndex = msg.indexOf(';');

            String ssid = msg.substring(0, separatorIndex);
            String password = msg.substring(separatorIndex + 1);

            LOG("[BLE] Received " + ssid + " with password " + password);

            stopConnectingToWifi();
            writeWifiConf(ssid, password);
            connectToWifi();
        }
    }
};

void initBle() {
    if (isBleInitialized) return;

    BLEDevice::init(getDeviceName().c_str());

    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    BLEService *pService = pServer->createService(BLEUUID(BLE_SERVICE_UUID), 20);

    pTxChar = pService->createCharacteristic(
        BLE_TX_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pTxChar->addDescriptor(new BLE2902());

    BLECharacteristic *pWifiConfChar = pService->createCharacteristic(
        BLE_WIFI_CONF_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pWifiConfChar->setCallbacks(new WifiConfCallbacks());

    pService->start();
    LOG("[BLE] Initialized as " + getDeviceName());

    isBleInitialized = true;
}

void startBleAdvertising() {
    if (isBleAdvertising) return;

    BLEDevice::getAdvertising()->start();
    isBleAdvertising = true;

    LOG("[BLE] Started advertising as " + getDeviceName());
}

void stopBleAdvertising() {
    if (!isBleAdvertising) return;

    BLEDevice::getAdvertising()->stop();
    isBleAdvertising = false;

    LOG("[BLE] Stopped advertising as " + getDeviceName());
}

String getDeviceName() {
    uint64_t chipId = ESP.getEfuseMac();
    String id = "";
    for (int i = 5; i >= 0; i--) {
        if ((chipId >> (i * 8) & 0xFF) < 16) id += "0";
        id += String((chipId >> (i * 8) & 0xFF), HEX);
    }
    id.toUpperCase();
    return "Alertoid-" + id;
}