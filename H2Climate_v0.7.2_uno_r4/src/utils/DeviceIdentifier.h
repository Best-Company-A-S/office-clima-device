#ifndef DEVICE_IDENTIFIER_H
#define DEVICE_IDENTIFIER_H

#include "../utils/FancyLog.h"
#include <Arduino.h>
#include <WiFiS3.h>

class DeviceIdentifier {
  public:
    DeviceIdentifier(FancyLog& fancyLog);
    static void initialize(); // Initialize the device identifier
    static String getDeviceId(); // Get the device ID as a string (lowercase hex format without colons)
	void printDeviceInfo(); // Print device information to Serial

  private:
    FancyLog& fancyLog;
    static String deviceId;
    static bool initialized;
    static void generateDeviceId(); // Generate device ID from MAC address
};

#endif // DEVICE_IDENTIFIER_H 