#include "DeviceIdentifier.h"

const int DEVICE_ID_ADDR = 0; // Address in EEPROM to store the deviceId
const int DEVICE_ID_MAX_LEN = 32; // Maximum length of the device ID string

DeviceIdentifier::DeviceIdentifier() {}

// Static member initialization
String DeviceIdentifier::deviceId = "";
bool DeviceIdentifier::initialized = false;

String DeviceIdentifier::getDeviceId() {
    if (!initialized) {
        initialize();
    }

    return deviceId;
}

void DeviceIdentifier::initialize() {
    // Try to load device ID from EEPROM
    //loadDeviceIdFromEEPROM();

    // If still empty after loading from EEPROM, generate a new one
    if (deviceId.isEmpty() || deviceId == "") {
        Serial.println("No device ID found in EEPROM. Generating a new one...");

		// Generate a new device ID based on the MAC address
        generateDeviceId();

        // Save the newly generated ID to EEPROM
        //saveDeviceIdToEEPROM();
    }

    initialized = true;
    Serial.println("Device Identifier Initialized. Device ID: " + deviceId);
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



void DeviceIdentifier::saveDeviceIdToEEPROM() {
    // Convert String to char array for EEPROM storage
    char idBuffer[DEVICE_ID_MAX_LEN];
    deviceId.toCharArray(idBuffer, DEVICE_ID_MAX_LEN);

    // Store the length first
    int len = deviceId.length();
    EEPROM.put(DEVICE_ID_ADDR, len);

    // Store the characters
    for (int i = 0; i < len; i++) {
        EEPROM.put(DEVICE_ID_ADDR + sizeof(int) + i, idBuffer[i]);
    }

    Serial.println("Device ID saved to EEPROM: " + deviceId);
}

void DeviceIdentifier::loadDeviceIdFromEEPROM() {
    // Read the length first
    int len;
    EEPROM.get(DEVICE_ID_ADDR, len);

    // Validate length to prevent issues
    if (len <= 0 || len >= DEVICE_ID_MAX_LEN) {
        deviceId = ""; // Invalid data in EEPROM
        Serial.println("Invalid device ID length in EEPROM");
        return;
    }

    // Read the characters
    char idBuffer[DEVICE_ID_MAX_LEN];
    for (int i = 0; i < len; i++) {
        EEPROM.get(DEVICE_ID_ADDR + sizeof(int) + i, idBuffer[i]);
    }
    idBuffer[len] = '\0'; // Null terminate

    // Convert to String
    deviceId = String(idBuffer);
    Serial.println("Device ID retrieved from EEPROM: " + deviceId);
}

/*
void DeviceIdentifier::saveDeviceIdToEEPROM() {
	EEPROM.put(DEVICE_ID_ADDR, deviceId); // Store the entire string in EEPROM
    Serial.println("Device ID saved to EEPROM: " + deviceId);
}

void DeviceIdentifier::loadDeviceIdFromEEPROM() {
	EEPROM.get(DEVICE_ID_ADDR, deviceId); // Retrieve the entire string from EEPROM
    Serial.println("Device ID retrieved from EEPROM: " + deviceId);
}
*/
