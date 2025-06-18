#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

#include <Arduino.h>
#include <EEPROM.h>

// Forward declaration
class FancyLog;

// Comment this line to disable EEPROM writes during testing
//#define ENABLE_EEPROM_WRITES

class EEPROMManager {
public:
    EEPROMManager(FancyLog& logger);

    // Initialize EEPROM
    void begin();

    // Clear all EEPROM data (set to 0xFF)
    void clearEEPROM();

    // Device ID methods
    bool saveDeviceId(const String& deviceId);
    bool loadDeviceId(String& deviceId);

    // WiFi credentials methods
    bool saveWiFiCredentials(const char* ssid, const char* password);
    bool loadWiFiCredentials(char* ssid, char* password);
    bool hasStoredWiFiCredentials();

    // Generic read/write methods
    template<typename T>
    bool write(int address, const T& value) {
        #ifdef ENABLE_EEPROM_WRITES
            EEPROM.put(address, value);
            logWriteOperation(address);
            return true;
        #else
            logSkippedWrite(address);
            return false;
        #endif
    }

    template<typename T>
    bool read(int address, T& value) {
        EEPROM.get(address, value);
        logReadOperation(address);
        return true;
    }

private:
    FancyLog& logger;

    // Helper methods for logging
    void logWriteOperation(int address);
    void logReadOperation(int address);
    void logSkippedWrite(int address);

    // EEPROM address map
    static const int WIFI_CRED_FLAG_ADDR = 0;    // 1 byte flag indicating if WiFi credentials are stored
    static const int WIFI_SSID_ADDR = 1;         // 33 bytes for SSID (32 + null terminator)
    static const int WIFI_PASS_ADDR = 34;        // 64 bytes for password (63 + null terminator)
    static const int DEVICE_ID_LEN_ADDR = 100;   // 4 bytes for device ID length (int)
    static const int DEVICE_ID_ADDR = 104;       // 32 bytes for device ID

    // Maximum lengths
    static const int MAX_SSID_LENGTH = 32;
    static const int MAX_PASSWORD_LENGTH = 63;
    static const int MAX_DEVICE_ID_LENGTH = 32;
};

#endif // EEPROM_MANAGER_H
