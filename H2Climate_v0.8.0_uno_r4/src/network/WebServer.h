#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "../config/Config.h"
#include "../utils/FancyLog.h"
#include "../network/NetworkManager.h"
#include <WiFiS3.h>
//#include <EEPROM.h>

class WebServer {
public:
    WebServer(FancyLog& log, NetworkManager& network);
    void begin();
    void handleClient();

private:
    FancyLog& fancyLog;
    NetworkManager& networkManager;
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
