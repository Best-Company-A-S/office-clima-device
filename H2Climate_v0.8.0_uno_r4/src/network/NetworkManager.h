#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "../config/Config.h"
#include "../network/OTAManager.h"
#include "../display/DisplayManager.h"
#include "../utils/FancyLog.h"

class NetworkManager {
  public:
    NetworkManager(FancyLog& fancyLog, OTAManager& otaManager, DisplayManager& display);
    void begin();
    bool connectWiFi();
    bool sendHttpPostRequest(String jsonPayload, String apiRoute);
    void checkForUpdates();
    void pollOTA();
    bool isConnected() { return WiFi.status() == WL_CONNECTED; }
    bool registerDevice(); // Method for device registration

  private:
    FancyLog& fancyLog;
    OTAManager& otaManager;
    DisplayManager& display;
    WiFiClient wifiClient;
    bool updateAvailable;
    String latestFirmwareVersion;
    bool handleUpdateResponse(String& response);
    bool downloadAndApplyUpdate(String& downloadUrl, int firmwareSize);
};

#endif // NETWORK_MANAGER_H
