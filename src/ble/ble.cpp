#include "ble.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "wifi/wifi.h"
#include "resource/resource.h"
#include "../debug.h"

#define FIRMWARE_VERSION "1.0.0"

bool isBleInitialized = false;
bool isBleDeviceConnected = false;
bool isBleAdvertising = false;

static BLECharacteristic *pTxChar;
static BLECharacteristic *pFirmwareChar;
static String getDeviceName();
static String getFirmwareJson();

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *s) {
        isBleDeviceConnected = true;
        pFirmwareChar->setValue(getFirmwareJson().c_str());
        LOG(F("[BLE] Device connected"));
    }

    void onDisconnect(BLEServer *s) {
        isBleDeviceConnected = false;
        LOG(F("[BLE] Device disconnected"));
        stopBleAdvertising();
    }
};

class WifiConfCallbacks : public BLECharacteristicCallbacks {
    // =====================
    // config format: {"ssid":"...","passphrase":"..."}
    // =====================
    void onWrite(BLECharacteristic *pChar) {
        String msg = pChar->getValue().c_str();

        if (msg.length() > 0) {
            JsonDocument doc;
            if (deserializeJson(doc, msg) != DeserializationError::Ok) {
                LOG(F("[BLE] WifiConf: invalid JSON"));
                return;
            }

            String ssid     = doc["ssid"].as<String>();
            String passphrase = doc["passphrase"].as<String>();

            LOG("[BLE] Received " + ssid + " with passphrase " + passphrase);

            stopConnectingToWifi();
            writeWifiConf(ssid, passphrase);
            connectToWifi();
        }
    }
};

class UrlConfCallbacks : public BLECharacteristicCallbacks {
    // =====================
    // config format: {"url":"...","code":200,"check_interval":30}
    // =====================
    void onWrite(BLECharacteristic *pChar) {
        String msg = pChar->getValue().c_str();

        if (msg.length() > 0) {
            JsonDocument doc;
            if (deserializeJson(doc, msg) != DeserializationError::Ok) {
                LOG(F("[BLE] UrlConf: invalid JSON"));
                return;
            }

            String url   = doc["url"].as<String>();
            int code     = doc["code"].as<int>();
            int interval = doc["check_interval"].as<int>() * 10;

            LOG("[BLE] Received url " + url + " code " + String(code) + " interval " + String(interval));

            writeResourceConf(url, code, interval);
        }
    }
};

class UrlConfReadCallbacks : public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic *pChar) {
        JsonDocument doc;
        doc["url"]            = getResourceUrl();
        doc["code"]           = getResourceExpectedCode();
        doc["check_interval"] = getResourceCheckInterval() / 10;

        String json;
        serializeJson(doc, json);
        pChar->setValue(json.c_str());
    }
};

class RebootCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
        String msg = pChar->getValue().c_str();
        if (msg == "reboot") {
            LOG(F("[BLE] Rebooting..."));
            ESP.restart();
        }
    }
};

class FirmwareCallbacks : public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic *pChar) {
        pChar->setValue(getFirmwareJson().c_str());
    }
};

void initBle() {
    if (isBleInitialized) return;

    BLEDevice::init(getDeviceName().c_str());

    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    BLEService *pService = pServer->createService(BLEUUID(BLE_SERVICE_UUID), 22);
    BLEDevice::getAdvertising()->addServiceUUID(BLE_SERVICE_UUID);

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

    BLECharacteristic *pUrlConfChar = pService->createCharacteristic(
        BLE_URL_CONF_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pUrlConfChar->setCallbacks(new UrlConfCallbacks());

    pFirmwareChar = pService->createCharacteristic(
        BLE_FIRMWARE_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    pFirmwareChar->setCallbacks(new FirmwareCallbacks());

    BLECharacteristic *pUrlConfReadChar = pService->createCharacteristic(
        BLE_URL_CONF_READ_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    pUrlConfReadChar->setCallbacks(new UrlConfReadCallbacks());

    BLECharacteristic *pRebootChar = pService->createCharacteristic(
        BLE_REBOOT_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pRebootChar->setCallbacks(new RebootCallbacks());

    pService->start();
    LOG("[BLE] Initialized as " + getDeviceName());

    isBleInitialized = true;
}

void deinitBle() {
    BLEDevice::deinit(true);
    LOG(F("[BLE] Deinitialized"));
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

static String getDeviceName() {
    uint64_t chipId = ESP.getEfuseMac();
    String id = "";
    for (int i = 5; i >= 0; i--) {
        if ((chipId >> (i * 8) & 0xFF) < 16) id += "0";
        id += String((chipId >> (i * 8) & 0xFF), HEX);
    }
    id.toUpperCase();
    return "Alertoid-" + id;
}

static String getFirmwareJson() {
    String mac = BLEDevice::getAddress().toString().c_str();
    mac.toUpperCase();
    String serial = getDeviceName();
    return "{\"firmware\":\"" + String(FIRMWARE_VERSION) + "\",\"mac\":\"" + mac + "\",\"serial\":\"" + serial + "\"}";
}
