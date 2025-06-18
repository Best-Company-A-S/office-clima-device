#ifndef DEVICE_IDENTIFIER_H
#define DEVICE_IDENTIFIER_H

#include "../config/Config.h"

// Forward declaration
class EEPROMManager;

class DeviceIdentifier {
public:
    DeviceIdentifier();
    static void initialize(); // Initialize the device identifier
    static String getDeviceId(); // Get the device ID as a string (lowercase hex format without colons)
    static void setEEPROMManager(EEPROMManager* manager); // Set the EEPROM manager instance

private:
    static String deviceId;
    static bool initialized;
    static EEPROMManager* eepromManager; // Add a pointer to EEPROMManager
    static void generateDeviceId(); // Generate device ID from MAC address
};

#endif // DEVICE_IDENTIFIER_H
