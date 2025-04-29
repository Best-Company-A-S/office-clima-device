#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <WiFiS3.h>

class OTAManager {
  public:
    static void begin(IPAddress localIP, const char* ssid, const char* password);
    static void poll();
    static bool beginUpdate(int size);
    static size_t write(const uint8_t* data, size_t len);
    static bool endUpdate();
    static void abortUpdate();
    static void applyUpdate();
};

#endif // OTA_MANAGER_H 