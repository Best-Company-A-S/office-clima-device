#include "EEPROMManager.h"
#include "../utils/FancyLog.h" // Include the FancyLog header here instead of in the .h file

EEPROMManager::EEPROMManager(FancyLog& log)
    : logger(log) {}

void EEPROMManager::begin() {
    logger.toSerial("EEPROM Manager initialized", DEBUG);
    EEPROM.begin();
}

void EEPROMManager::clearEEPROM() {
    #ifdef ENABLE_EEPROM_WRITES
        logger.toSerial("Clearing all EEPROM data", INFO);
        for (int i = 0; i < EEPROM.length(); i++) {
            EEPROM.write(i, 0xFF); // Default empty value for EEPROM
        }
    #else
        logger.toSerial("EEPROM clear skipped (disabled for testing)", INFO);
    #endif
}

// Logging helper methods
void EEPROMManager::logWriteOperation(int address) {
    logger.toSerial("EEPROM: Written to address " + String(address), DEBUG);
}

void EEPROMManager::logReadOperation(int address) {
    logger.toSerial("EEPROM: Read from address " + String(address), DEBUG);
}

void EEPROMManager::logSkippedWrite(int address) {
    logger.toSerial("EEPROM: Write to address " + String(address) + " skipped (disabled for testing)", INFO);
}

// Device ID methods
bool EEPROMManager::saveDeviceId(const String& deviceId) {
    if (deviceId.length() > MAX_DEVICE_ID_LENGTH) {
        logger.toSerial("Device ID too long to save in EEPROM", ERROR);
        return false;
    }

    #ifdef ENABLE_EEPROM_WRITES
        // Convert String to char array for EEPROM storage
        char idBuffer[MAX_DEVICE_ID_LENGTH + 1];
        deviceId.toCharArray(idBuffer, MAX_DEVICE_ID_LENGTH + 1);

        // Store the length first
        int len = deviceId.length();
        write(DEVICE_ID_LEN_ADDR, len);

        // Store the characters
        for (int i = 0; i < len; i++) {
            write(DEVICE_ID_ADDR + i, idBuffer[i]);
        }

        logger.toSerial("Device ID saved to EEPROM: " + deviceId, INFO);
        return true;
    #else
        logger.toSerial("Device ID save skipped (EEPROM writes disabled for testing): " + deviceId, INFO);
        return false;
    #endif
}

bool EEPROMManager::loadDeviceId(String& deviceId) {
    // Read the length first
    int len;
    read(DEVICE_ID_LEN_ADDR, len);

    // Validate length to prevent issues
    if (len <= 0 || len > MAX_DEVICE_ID_LENGTH) {
        deviceId = ""; // Invalid data in EEPROM
        logger.toSerial("Invalid device ID length in EEPROM", WARNING);
        return false;
    }

    // Read the characters
    char idBuffer[MAX_DEVICE_ID_LENGTH + 1];
    for (int i = 0; i < len; i++) {
        char c;
        read(DEVICE_ID_ADDR + i, c);
        idBuffer[i] = c;
    }
    idBuffer[len] = '\0'; // Ensure null termination

    // Convert to String
    deviceId = String(idBuffer);
    logger.toSerial("Device ID retrieved from EEPROM: " + deviceId, INFO);
    return true;
}

// WiFi credentials methods
bool EEPROMManager::saveWiFiCredentials(const char* ssid, const char* password) {
    #ifdef ENABLE_EEPROM_WRITES
        // Set flag indicating credentials are stored
        write(WIFI_CRED_FLAG_ADDR, (byte)1);

        // Save SSID (max 32 chars + null terminator)
        for (int i = 0; i < MAX_SSID_LENGTH + 1; i++) {
            write(WIFI_SSID_ADDR + i, ssid[i]);
            if (ssid[i] == '\0') break;
        }

        // Save password (max 63 chars + null terminator)
        for (int i = 0; i < MAX_PASSWORD_LENGTH + 1; i++) {
            write(WIFI_PASS_ADDR + i, password[i]);
            if (password[i] == '\0') break;
        }

        logger.toSerial("WiFi credentials saved to EEPROM for SSID: " + String(ssid), INFO);
        return true;
    #else
        logger.toSerial("WiFi credentials save skipped (EEPROM writes disabled for testing) for SSID: " + String(ssid), INFO);
        return false;
    #endif
}

bool EEPROMManager::loadWiFiCredentials(char* ssid, char* password) {
    // Check if credentials are stored
    byte flag;
    read(WIFI_CRED_FLAG_ADDR, flag);
    if (flag != 1) {
        logger.toSerial("No WiFi credentials found in EEPROM", DEBUG);
        return false;
    }

    // Load SSID
    for (int i = 0; i < MAX_SSID_LENGTH + 1; i++) {
        char c;
        read(WIFI_SSID_ADDR + i, c);
        ssid[i] = c;
        if (c == '\0') break;
    }
    ssid[MAX_SSID_LENGTH] = '\0'; // Ensure null termination

    // Load password
    for (int i = 0; i < MAX_PASSWORD_LENGTH + 1; i++) {
        char c;
        read(WIFI_PASS_ADDR + i, c);
        password[i] = c;
        if (c == '\0') break;
    }
    password[MAX_PASSWORD_LENGTH] = '\0'; // Ensure null termination

    logger.toSerial("WiFi credentials loaded from EEPROM for SSID: " + String(ssid), INFO);
    return true;
}

bool EEPROMManager::hasStoredWiFiCredentials() {
    byte flag;
    read(WIFI_CRED_FLAG_ADDR, flag);
    return flag == 1;
}
