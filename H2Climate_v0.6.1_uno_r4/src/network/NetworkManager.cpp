#include "NetworkManager.h"
#include "OTAManager.h"

// Initialize static members
WiFiUDP NetworkManager::ntpUDP;
const char* NetworkManager::ntpServer = "pool.ntp.org";

NetworkManager::NetworkManager(DisplayManager& display, Logger& logger)
    : display(display), logger(logger), updateAvailable(false) {}

void NetworkManager::begin() {
    // Initialize UDP socket for NTP
    ntpUDP.begin(5757);
    
    // Connect to WiFi
    connectWiFi();
    
    // Sync time using NTP
    setSyncProvider(NetworkManager::getNtpTime);
    if (timeStatus() == timeSet) {
        logger.log("Time synchronized");
    } else {
        logger.log("Failed to synchronize time");
    }
    
    // Initialize OTA
    OTAManager::begin(WiFi.localIP(), WIFI_SSID, WIFI_PASS);
}

void NetworkManager::pollOTA() {
    OTAManager::poll();
}

bool NetworkManager::connectWiFi() {
    logger.logWithBorder("Connecting to WiFi: " + String(WIFI_SSID));
    display.showNeutralFace();
    
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
        delay(1000);
    }
    
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    unsigned long startAttemptTime = millis();
    int attempts = 0;
    
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT) {
        Serial.print(".");
        delay(1000);
        
        if (attempts++ > 5) {
            WiFi.disconnect();
            delay(500);
            WiFi.begin(WIFI_SSID, WIFI_PASS);
            attempts = 0;
        }
    }
    
    // Wait for valid IP address
    startAttemptTime = millis();
    while (WiFi.localIP()[0] == 0 && millis() - startAttemptTime < WIFI_TIMEOUT) {
        delay(500);
    }
    
    if (WiFi.status() == WL_CONNECTED && WiFi.localIP()[0] != 0) {
        logger.logWithBorder("WiFi connected, IP: " + WiFi.localIP().toString());
        display.showHappyFace();
        return true;
    } else {
        logger.logWithBorder("WiFi connection failed");
        display.showSadFace();
        return false;
    }
}

bool NetworkManager::sendHttpPostRequest(String jsonPayload, String apiRoute) {
    int apiAttempts = 0;
    bool success = false;
    
    while (apiAttempts < MAX_API_ATTEMPTS && !success) {
        if (apiAttempts > 0) {
            logger.log("Retry attempt " + String(apiAttempts) + " of " + String(MAX_API_ATTEMPTS - 1));
            display.showRetryAnimation();
        }
        
        if (!isConnected()) {
            logger.log("WiFi not connected. Reconnecting...");
            display.showSadFace();
            if (!connectWiFi()) {
                return false;
            }
        }
        
        display.showNeutralFace();
        if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
            logger.log("Failed to connect to server");
            apiAttempts++;
            continue;
        }
        
        String httpRequest =
            "POST " + apiRoute + " HTTP/1.1\r\n" +
            "Host: " + String(SERVER_URL) + "\r\n" +
            "Content-Type: application/json\r\n" +
            "Content-Length: " + String(jsonPayload.length()) + "\r\n" +
            "Connection: close\r\n\r\n" +
            jsonPayload;
        
        wifiClient.print(httpRequest);
        
        unsigned long timeout = millis();
        while (millis() - timeout < API_TIMEOUT) {
            if (wifiClient.available()) {
                String response = wifiClient.readString();
                wifiClient.stop();
                
                if (response.indexOf("200 OK") > 0 || response.indexOf("201 Created") > 0) {
                    logger.log("Data sent successfully to " + apiRoute);
                    display.showHappyFace();
                    return true;
                }
                break;
            }
            delay(10);
        }
        
        wifiClient.stop();
        apiAttempts++;
    }
    
    display.showSadFace();
    return false;
}

time_t NetworkManager::getNtpTime() {
    const int NTP_PACKET_SIZE = 48;
    byte packetBuffer[NTP_PACKET_SIZE];
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    
    packetBuffer[0] = 0b11100011;
    packetBuffer[1] = 0;
    packetBuffer[2] = 6;
    packetBuffer[3] = 0xEC;
    
    // Try multiple NTP servers if the first one fails
    const char* ntpServers[] = {"pool.ntp.org", "time.nist.gov", "time.google.com"};
    const int numServers = 3;
    
    for (int server = 0; server < numServers; server++) {
        ntpUDP.beginPacket(ntpServers[server], 123);
        ntpUDP.write(packetBuffer, NTP_PACKET_SIZE);
        ntpUDP.endPacket();
        
        // Wait up to 5 seconds for response
        unsigned long startWait = millis();
        while (millis() - startWait < 5000) {
            if (ntpUDP.parsePacket()) {
                ntpUDP.read(packetBuffer, NTP_PACKET_SIZE);
                unsigned long secsSince1900 = (unsigned long)packetBuffer[40] << 24 |
                                            (unsigned long)packetBuffer[41] << 16 |
                                            (unsigned long)packetBuffer[42] << 8 |
                                            (unsigned long)packetBuffer[43];
                const unsigned long seventyYears = 2208988800UL;
                unsigned long epoch = secsSince1900 - seventyYears;
                return epoch;
            }
            delay(10);
        }
    }
    return 0;
}

void NetworkManager::checkForUpdates() {
    logger.logWithBorder("Checking for firmware updates...");
    logger.log("Current version: " + String(FIRMWARE_VERSION));
    display.showNeutralFace();
    
    if (!isConnected()) {
        logger.log("WiFi not connected, skipping update check");
        display.showSadFace();
        return;
    }
    
    String updateUrl = "/api/firmware/check?deviceId=" + String(DEVICE_ID) +
                      "&currentVersion=" + String(FIRMWARE_VERSION) +
                      "&modelType=" + String(MODEL_TYPE);
    
    if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
        logger.log("Failed to connect to update server");
        display.showSadFace();
        return;
    }
    
    String httpRequest =
        "GET " + updateUrl + " HTTP/1.1\r\n" +
        "Host: " + String(SERVER_URL) + "\r\n" +
        "Connection: close\r\n\r\n";
    
    wifiClient.print(httpRequest);
    
    // Read the entire response
    String response = "";
    unsigned long timeout = millis();
    bool responseReceived = false;
    
    while (millis() - timeout < API_TIMEOUT) {
        if (wifiClient.available()) {
            response += (char)wifiClient.read();
            responseReceived = true;
        }
        
        if (!wifiClient.connected() && !wifiClient.available() && responseReceived) {
            break;
        }
        
        delay(1);
    }
    
    wifiClient.stop();
    
    if (!responseReceived) {
        logger.log("No response received from server");
        display.showSadFace();
        return;
    }
    
    // Extract JSON from response
    int jsonStart = response.indexOf("\r\n\r\n");
    if (jsonStart > -1) {
        String jsonResponse = response.substring(jsonStart + 4);
        jsonResponse.trim();
        
        // Find the actual JSON object
        int firstBrace = jsonResponse.indexOf('{');
        int lastBrace = jsonResponse.lastIndexOf('}');
        if (firstBrace > -1 && lastBrace > -1) {
            jsonResponse = jsonResponse.substring(firstBrace, lastBrace + 1);
            
            if (handleUpdateResponse(jsonResponse)) {
                return;
            }
        }
    }
    
    logger.log("Update check failed");
    display.showSadFace();
}

bool NetworkManager::handleUpdateResponse(String& jsonBody) {
    StaticJsonDocument<512> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, jsonBody);
    
    if (error) {
        logger.log("JSON Parse Error: " + String(error.c_str()));
        return false;
    }
    
    updateAvailable = jsonDoc["updateAvailable"].as<bool>();
    
    if (!updateAvailable) {
        String message = jsonDoc["message"] | "No updates available";
        logger.log("Update Status: " + message);
        display.showHappyFace();
        return true;
    }
    
    latestFirmwareVersion = jsonDoc["latestVersion"].as<String>();
    String downloadUrl = "/api/firmware/download?deviceId=" + String(DEVICE_ID) +
                        "&version=" + latestFirmwareVersion +
                        "&modelType=" + String(MODEL_TYPE);
    
    int firmwareSize = jsonDoc["size"] | 0;
    if (firmwareSize <= 0) {
        logger.log("Invalid firmware size");
        return false;
    }
    
    return downloadAndApplyUpdate(downloadUrl, firmwareSize);
}

bool NetworkManager::downloadAndApplyUpdate(String& downloadUrl, int firmwareSize) {
    if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
        logger.log("Failed to connect to download server");
        return false;
    }
    
    String downloadRequest =
        "GET " + downloadUrl + " HTTP/1.1\r\n" +
        "Host: " + String(SERVER_URL) + "\r\n" +
        "Connection: close\r\n\r\n";
    
    wifiClient.print(downloadRequest);
    
    // Skip headers
    while (wifiClient.available()) {
        String line = wifiClient.readStringUntil('\n');
        if (line == "\r") break;
    }
    
    // Initialize OTA update
    if (OTAManager::beginUpdate(firmwareSize)) {
        logger.log("Started firmware update process");
        int totalRead = 0;
        
        while (wifiClient.available() && totalRead < firmwareSize) {
            uint8_t buffer[128];
            int bytesRead = wifiClient.read(buffer, sizeof(buffer));
            if (bytesRead > 0) {
                if (OTAManager::write(buffer, bytesRead) != bytesRead) {
                    logger.log("Error writing firmware data");
                    OTAManager::abortUpdate();
                    wifiClient.stop();
                    return false;
                }
                totalRead += bytesRead;
                
                if (totalRead % 1024 == 0) {
                    logger.log("Downloaded: " + String(totalRead) + " bytes");
                }
            }
        }
        
        if (totalRead == firmwareSize && OTAManager::endUpdate()) {
            logger.log("Firmware downloaded successfully!");
            logger.log("Applying update...");
            
            // Notify server about the update
            StaticJsonDocument<256> statusDoc;
            statusDoc["deviceId"] = DEVICE_ID;
            statusDoc["firmwareStatus"] = "UPDATING";
            statusDoc["currentVersion"] = FIRMWARE_VERSION;
            statusDoc["targetVersion"] = latestFirmwareVersion;
            
            String statusData;
            serializeJson(statusDoc, statusData);
            sendHttpPostRequest(statusData, "/api/devices/" + String(DEVICE_ID));
            
            delay(1000);
            OTAManager::applyUpdate();  // This will restart the board
            return true;
        }
    }
    
    wifiClient.stop();
    return false;
} 