#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "../config/Config.h"
#include "../network/OTAManager.h"
#include "../display/DisplayManager.h"
#include "../utils/FancyLog.h"
#include "../utils/EEPROMManager.h"

class NetworkManager {
  public:
    NetworkManager(FancyLog& fancyLog, OTAManager& otaManager, DisplayManager& display);
    void setEEPROMManager(EEPROMManager* manager); // Add method to set the EEPROM manager
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
    EEPROMManager* eepromManager; // Add reference to the EEPROM manager
    WiFiClient wifiClient;
    bool updateAvailable;
    String latestFirmwareVersion;
    bool handleUpdateResponse(String& response);
    bool downloadAndApplyUpdate(String& downloadUrl, int firmwareSize);
};

#endif // NETWORK_MANAGER_H
