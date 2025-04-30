#include "DeviceIdentifier.h"

DeviceIdentifier::DeviceIdentifier(FancyLog& fancyLog)
    : fancyLog(fancyLog) {}

// Static member initialization
String DeviceIdentifier::deviceId = "";
bool DeviceIdentifier::initialized = false;

void DeviceIdentifier::initialize() {
    if (!initialized) {
        generateDeviceId();
        initialized = true;
    }
}

String DeviceIdentifier::getDeviceId() {
    if (!initialized) {
        initialize();
    }
    return deviceId;
}

void DeviceIdentifier::printDeviceInfo() {
    if (!initialized) {
        initialize();
    }
	fancyLog.toSerial("Device ID (MAC-based): " + deviceId, INFO);
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