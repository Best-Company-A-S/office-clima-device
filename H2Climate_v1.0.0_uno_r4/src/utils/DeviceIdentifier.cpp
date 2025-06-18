#include "DeviceIdentifier.h"
#include "EEPROMManager.h" // Include the header here in the implementation file

// Static member initialization
String DeviceIdentifier::deviceId = "";
bool DeviceIdentifier::initialized = false;
EEPROMManager* DeviceIdentifier::eepromManager = nullptr;

DeviceIdentifier::DeviceIdentifier() {}

String DeviceIdentifier::getDeviceId() {
    if (!initialized) {
        initialize();
    }

    return deviceId;
}

void DeviceIdentifier::setEEPROMManager(EEPROMManager* manager) {
    eepromManager = manager;
}

void DeviceIdentifier::initialize() {
    if (eepromManager != nullptr) {
        // Try to load device ID from EEPROM
        if (eepromManager->loadDeviceId(deviceId)) {
            Serial.println("Device ID loaded from EEPROM: " + deviceId);
        } else {
            Serial.println("No valid device ID found in EEPROM. Generating a new one");
            generateDeviceId(); // Generate a new device ID based on the MAC address

            // Save the newly generated ID to EEPROM
            eepromManager->saveDeviceId(deviceId);
        }
    } else {
        // No EEPROM manager available, just generate a device ID without storing it
        Serial.println("No EEPROM manager available, generating temporary device ID");
        generateDeviceId();
    }

    initialized = true;
}

void DeviceIdentifier::generateDeviceId() {
    byte mac[6];
    WiFi.macAddress(mac);
    
    // Format MAC address as a continuous lowercase hex string
    deviceId = "";
    for (int i = 0; i < 6; i++) {
        if (mac[i] < 0x10) {
            deviceId += "0";
        }
        deviceId += String(mac[i], HEX);
    }
    
    // Ensure lowercase formatting
    deviceId.toLowerCase();
}
