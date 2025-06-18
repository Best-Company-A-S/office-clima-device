#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "../config/Config.h"
#include "../utils/FancyLog.h"
#include "../network/NetworkManager.h"
#include "../utils/EEPROMManager.h"
#include <WiFiS3.h>

class WebServer {
public:
    WebServer(FancyLog& log, NetworkManager& network);
    void setEEPROMManager(EEPROMManager* manager); // Add method to set EEPROM manager
    void begin();
    void handleClient();

private:
    FancyLog& fancyLog;
    NetworkManager& networkManager;
    EEPROMManager* eepromManager; // Add reference to the EEPROM manager
    WiFiServer server;

    void handleRoot(WiFiClient& client);
    void handleSettings(WiFiClient& client);
    void handleSaveSettings(WiFiClient& client, String request);
    void handleNotFound(WiFiClient& client);
    void sendHeader(WiFiClient& client, const char* title);
    void sendFooter(WiFiClient& client);
    String getContentType(String filename);
    String urlDecode(String text);
    void saveNetworkCredentials(const char* ssid, const char* password);
};

#endif // WEB_SERVER_H
