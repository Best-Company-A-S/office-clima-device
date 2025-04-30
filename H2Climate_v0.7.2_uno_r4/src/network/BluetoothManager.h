#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include "../config/Config.h"
#include "../display/DisplayManager.h"
#include "../utils/FancyLog.h"
#include <ArduinoBLE.h>
#include <EEPROM.h>

// Define EEPROM address for stored WiFi credentials
#define WIFI_CRED_START_ADDR 0
#define WIFI_SSID_MAX_LENGTH 32
#define WIFI_PASS_MAX_LENGTH 64
#define BLUETOOTH_TIMEOUT 60000 // 60 seconds timeout for BLE provisioning

class BluetoothManager {
public:
    BluetoothManager(DisplayManager& display, FancyLog& fancyLog);
    void begin();
    bool hasStoredWiFiCredentials();
    void enableProvisioningMode();
    void disableProvisioningMode();
    bool isProvisioningActive();
    void pollBLE();
    bool getStoredWiFiCredentials(char* ssid, char* password);

private:
    DisplayManager& display;
    FancyLog& fancyLog;
    bool provisioningActive;
    unsigned long provisioningStartTime;
    
    // BLE Service and Characteristics
    BLEService wifiProvisionService;
    BLECharacteristic ssidCharacteristic;
    BLECharacteristic passwordCharacteristic;
    BLECharacteristic statusCharacteristic;
    
    void saveWiFiCredentials(const char* ssid, const char* password);
    void clearStoredCredentials();
    void onBLEConnected(BLEDevice central);
    void onBLEDisconnected(BLEDevice central);
    void handleSSIDWrite(BLEDevice central, BLECharacteristic characteristic);
    void handlePasswordWrite(BLEDevice central, BLECharacteristic characteristic);
};

#endif // BLUETOOTH_MANAGER_H 