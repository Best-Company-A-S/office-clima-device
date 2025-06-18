#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "../config/Config.h"
#include "../network/OTAManager.h"
#include "../display/DisplayManager.h"
#include "../utils/FancyLog.h"
#include <EEPROM.h>

class NetworkManager {
  public:
    NetworkManager(FancyLog& fancyLog, OTAManager& otaManager, DisplayManager& display);
    void begin();
    bool connectWiFi();
    bool connectWiFi(const char* ssid, const char* password);  // Overloaded method for dynamic credentials
    bool sendHttpPostRequest(String jsonPayload, String apiRoute);
    void checkForUpdates();
    void pollOTA();
    bool isConnected() { return WiFi.status() == WL_CONNECTED; }
    bool registerDevice(); // Method for device registration
    void saveWiFiCredentials(const char* ssid, const char* password); // Save to EEPROM
    bool loadWiFiCredentials(char* ssid, char* password); // Load from EEPROM
    bool hasStoredCredentials(); // Check if credentials exist

  private:
    FancyLog& fancyLog;
    OTAManager& otaManager;
    DisplayManager& display;
    WiFiClient wifiClient;
    bool updateAvailable;
    String latestFirmwareVersion;
    bool handleUpdateResponse(String& response);
    bool downloadAndApplyUpdate(String& downloadUrl, int firmwareSize);

    // EEPROM address definitions
    static const int EEPROM_WIFI_FLAG = 0;     // 1 byte flag to indicate if credentials are stored
    static const int EEPROM_SSID_ADDR = 1;     // Start address for SSID (33 bytes max)
    static const int EEPROM_PASS_ADDR = 34;    // Start address for password (64 bytes max)
};

#endif // NETWORK_MANAGER_H
