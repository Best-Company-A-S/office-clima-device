#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <ArduinoBLE.h>
#include "../utils/FancyLog.h"
#include "../config/Config.h"

class BluetoothManager {
private:
    FancyLog& logger;
    BLEService wifiConfigService;
    BLEStringCharacteristic ssidCharacteristic;
    BLEStringCharacteristic passwordCharacteristic;
    BLEBoolCharacteristic confirmCharacteristic;

    char ssidBuffer[33]; // SSID can be up to 32 characters + null terminator
    char passwordBuffer[64]; // Password can be up to 63 characters + null terminator
    bool credentialsReceived;

    void setupBLEService();

    // Static callback for BLE characteristic
    static void confirmCharacteristicWrittenCallback(BLEDevice central, BLECharacteristic characteristic);

    // Store instance pointer to access from static callback
    static BluetoothManager* _instance;

    // Internal handler for confirm characteristic being written
    void handleConfirmCharacteristicWritten();

public:
    BluetoothManager(FancyLog& log);
    bool begin();
    void poll();
    bool hasWifiCredentials();
    void getWifiCredentials(char* ssid, char* password);
    void resetCredentialFlag();
};

#endif
