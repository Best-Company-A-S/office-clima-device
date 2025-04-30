#include "BluetoothManager.h"

// UUID for WiFi Provision Service and Characteristics
// Use standard BLE UUIDs in the format 00000000-0000-0000-0000-000000000000
#define WIFI_SERVICE_UUID              "180A"  // Device Information Service
#define SSID_CHARACTERISTIC_UUID       "2A00"  // Device Name
#define PASSWORD_CHARACTERISTIC_UUID   "2A01"  // Appearance
#define STATUS_CHARACTERISTIC_UUID     "2A05"  // Service Changed

BluetoothManager::BluetoothManager(DisplayManager& display, FancyLog& fancyLog)
    : display(display), 
      fancyLog(fancyLog),
      provisioningActive(false),
      provisioningStartTime(0),
      wifiProvisionService(WIFI_SERVICE_UUID),
      ssidCharacteristic(SSID_CHARACTERISTIC_UUID, BLERead | BLEWrite, WIFI_SSID_MAX_LENGTH),
      passwordCharacteristic(PASSWORD_CHARACTERISTIC_UUID, BLEWrite, WIFI_PASS_MAX_LENGTH),
      statusCharacteristic(STATUS_CHARACTERISTIC_UUID, BLERead | BLENotify, 20) {}

void BluetoothManager::begin() {
    // Initialize EEPROM
    EEPROM.begin(WIFI_SSID_MAX_LENGTH + WIFI_PASS_MAX_LENGTH + 2);
    
    // Initialize BLE
    if (!BLE.begin()) {
        fancyLog.toSerial("Failed to initialize BLE!", ERROR);
        return;
    }
    
    // Check if we have stored credentials
    if (!hasStoredWiFiCredentials()) {
        fancyLog.toSerial("No WiFi credentials found, entering provisioning mode", INFO);
        enableProvisioningMode();
    } else {
        fancyLog.toSerial("Found stored WiFi credentials", INFO);
    }
}

bool BluetoothManager::hasStoredWiFiCredentials() {
    uint8_t flag = EEPROM.read(WIFI_CRED_START_ADDR);
    return flag == 1;
}

void BluetoothManager::enableProvisioningMode() {
    if (provisioningActive) {
        return;
    }
    
    display.showBluetoothIcon();
    fancyLog.toSerial("Entering BLE WiFi provisioning mode", INFO);
    
    // Set up the BLE device
    BLE.setLocalName("H2Climate");
    BLE.setDeviceName("H2Climate");
    
    // Set up BLE event handlers
    BLE.setEventHandler(BLEConnected, [this](BLEDevice central) {
        this->onBLEConnected(central);
    });
    
    BLE.setEventHandler(BLEDisconnected, [this](BLEDevice central) {
        this->onBLEDisconnected(central);
    });
    
    // Set up characteristics
    ssidCharacteristic.setEventHandler(BLEWritten, [this](BLEDevice central, BLECharacteristic characteristic) {
        this->handleSSIDWrite(central, characteristic);
    });
    
    passwordCharacteristic.setEventHandler(BLEWritten, [this](BLEDevice central, BLECharacteristic characteristic) {
        this->handlePasswordWrite(central, characteristic);
    });
    
    // Initial values
    statusCharacteristic.writeValue("Ready");
    
    // Add characteristics to the service
    wifiProvisionService.addCharacteristic(ssidCharacteristic);
    wifiProvisionService.addCharacteristic(passwordCharacteristic);
    wifiProvisionService.addCharacteristic(statusCharacteristic);
    
    // Add service to BLE
    BLE.addService(wifiProvisionService);
    
    // Start advertising
    BLE.advertise();
    
    provisioningActive = true;
    provisioningStartTime = millis();
    
    fancyLog.toSerial("BLE advertising started. Connect with Expo app to provide WiFi credentials.", INFO);
}

void BluetoothManager::disableProvisioningMode() {
    if (!provisioningActive) {
        return;
    }
    
    BLE.stopAdvertise();
    BLE.disconnect();
    
    provisioningActive = false;
    fancyLog.toSerial("BLE provisioning mode disabled", INFO);
    display.showNeutralFace();
}

bool BluetoothManager::isProvisioningActive() {
    // Check timeout condition
    if (provisioningActive && millis() - provisioningStartTime > BLUETOOTH_TIMEOUT) {
        fancyLog.toSerial("BLE provisioning timeout", WARNING);
        disableProvisioningMode();
        return false;
    }
    
    return provisioningActive;
}

void BluetoothManager::pollBLE() {
    if (provisioningActive) {
        BLE.poll();
    }
}

bool BluetoothManager::getStoredWiFiCredentials(char* ssid, char* password) {
    if (!hasStoredWiFiCredentials()) {
        return false;
    }
    
    // Read the SSID length and SSID
    uint8_t ssidLength = EEPROM.read(WIFI_CRED_START_ADDR + 1);
    if (ssidLength > WIFI_SSID_MAX_LENGTH) {
        return false;
    }
    
    for (int i = 0; i < ssidLength; i++) {
        ssid[i] = EEPROM.read(WIFI_CRED_START_ADDR + 2 + i);
    }
    ssid[ssidLength] = '\0';
    
    // Read the password length and password
    uint8_t passLength = EEPROM.read(WIFI_CRED_START_ADDR + 2 + ssidLength);
    if (passLength > WIFI_PASS_MAX_LENGTH) {
        return false;
    }
    
    for (int i = 0; i < passLength; i++) {
        password[i] = EEPROM.read(WIFI_CRED_START_ADDR + 3 + ssidLength + i);
    }
    password[passLength] = '\0';
    
    return true;
}

void BluetoothManager::saveWiFiCredentials(const char* ssid, const char* password) {
    uint8_t ssidLength = strlen(ssid);
    uint8_t passLength = strlen(password);
    
    if (ssidLength > WIFI_SSID_MAX_LENGTH || passLength > WIFI_PASS_MAX_LENGTH) {
        fancyLog.toSerial("WiFi credentials too long", ERROR);
        return;
    }
    
    // Save flag to indicate we have stored credentials
    EEPROM.write(WIFI_CRED_START_ADDR, 1);
    
    // Save SSID length and SSID
    EEPROM.write(WIFI_CRED_START_ADDR + 1, ssidLength);
    for (int i = 0; i < ssidLength; i++) {
        EEPROM.write(WIFI_CRED_START_ADDR + 2 + i, ssid[i]);
    }
    
    // Save password length and password
    EEPROM.write(WIFI_CRED_START_ADDR + 2 + ssidLength, passLength);
    for (int i = 0; i < passLength; i++) {
        EEPROM.write(WIFI_CRED_START_ADDR + 3 + ssidLength + i, password[i]);
    }
    
    EEPROM.commit();
    fancyLog.toSerial("WiFi credentials saved to EEPROM", INFO);
}

void BluetoothManager::clearStoredCredentials() {
    EEPROM.write(WIFI_CRED_START_ADDR, 0);
    EEPROM.commit();
    fancyLog.toSerial("WiFi credentials cleared", INFO);
}

void BluetoothManager::onBLEConnected(BLEDevice central) {
    fancyLog.toSerial("BLE connection from: " + String(central.address()), INFO);
    statusCharacteristic.writeValue("Connected");
}

void BluetoothManager::onBLEDisconnected(BLEDevice central) {
    fancyLog.toSerial("BLE disconnected: " + String(central.address()), INFO);
}

void BluetoothManager::handleSSIDWrite(BLEDevice central, BLECharacteristic characteristic) {
    String ssid = String((char*)characteristic.value());
    fancyLog.toSerial("Received SSID: " + ssid, INFO);
    statusCharacteristic.writeValue("SSID received");
}

void BluetoothManager::handlePasswordWrite(BLEDevice central, BLECharacteristic characteristic) {
    String password = String((char*)characteristic.value());
    fancyLog.toSerial("Received Password. Length: " + String(password.length()), INFO);
    
    // Get the last received SSID
    char ssidBuffer[WIFI_SSID_MAX_LENGTH + 1];
    ssidCharacteristic.readValue(ssidBuffer, WIFI_SSID_MAX_LENGTH);
    ssidBuffer[WIFI_SSID_MAX_LENGTH] = '\0';
    
    // Save credentials
    saveWiFiCredentials(ssidBuffer, password.c_str());
    
    // Update status and disable provisioning
    statusCharacteristic.writeValue("Credentials saved");
    disableProvisioningMode();
} 