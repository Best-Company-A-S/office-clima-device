#ifndef DEVICE_IDENTIFIER_H
#define DEVICE_IDENTIFIER_H

#include "../config/Config.h"

class DeviceIdentifier {
  public:
    DeviceIdentifier();
    static void initialize(); // Initialize the device identifier
    static String getDeviceId(); // Get the device ID as a string (lowercase hex format without colons)

  private:
    static String deviceId;
    static bool initialized;
    static void generateDeviceId(); // Generate device ID from MAC address
};

#endif // DEVICE_IDENTIFIER_H 