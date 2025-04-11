#ifndef DEVICE_IDENTIFIER_H
#define DEVICE_IDENTIFIER_H

#include <Arduino.h>
#include <WiFiS3.h>

class DeviceIdentifier {
public:
    // Initialize the device identifier
    static void initialize();
    
    // Get the device ID as a string (lowercase hex format without colons)
    static String getDeviceId();
    
    // Print device information to Serial
    static void printDeviceInfo();

private:
    static String deviceId;
    static bool initialized;
    
    // Generate device ID from MAC address
    static void generateDeviceId();
};

#endif // DEVICE_IDENTIFIER_H 