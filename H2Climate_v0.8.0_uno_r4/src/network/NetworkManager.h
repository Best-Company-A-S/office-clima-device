#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "../config/Config.h"
#include "../network/OTAManager.h"
#include "../display/DisplayManager.h"
//#include "../utils/FancyLog.h"
//#include "OTAManager.h"

class NetworkManager {
  public:
    NetworkManager(DisplayManager& display, FancyLog& fancyLog);
    void begin();
    bool connectWiFi();
    bool sendHttpPostRequest(String jsonPayload, String apiRoute);
    void checkForUpdates();
    void pollOTA();
    bool isConnected() { return WiFi.status() == WL_CONNECTED; }

  private:
    DisplayManager& display;
    FancyLog& fancyLog;
    WiFiClient wifiClient;
    bool updateAvailable;
    String latestFirmwareVersion;
    bool handleUpdateResponse(String& response);
    bool downloadAndApplyUpdate(String& downloadUrl, int firmwareSize);
};

#endif // NETWORK_MANAGER_H 