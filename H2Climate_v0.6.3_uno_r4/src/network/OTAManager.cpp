#include "OTAManager.h"
#include <ArduinoOTA.h>

void OTAManager::begin(IPAddress localIP, const char* ssid, const char* password) {
    ArduinoOTA.begin(localIP, ssid, password, InternalStorage);
}

void OTAManager::poll() {
    ArduinoOTA.poll();
}

bool OTAManager::beginUpdate(int size) {
    return InternalStorage.open(size);
}

size_t OTAManager::write(const uint8_t* data, size_t len) {
    size_t written = 0;
    for (size_t i = 0; i < len; i++) {
        if (InternalStorage.write(data[i]) != 1) {
            break;
        }
        written++;
    }
    return written;
}

bool OTAManager::endUpdate() {
    InternalStorage.close();
    return true;
}

void OTAManager::abortUpdate() {
    // No direct abort function in InternalStorage, but we can close it
    InternalStorage.close();
}

void OTAManager::applyUpdate() {
    InternalStorage.apply();
} 