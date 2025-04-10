#include "NetworkManager.h"
#include "OTAManager.h"
#include "../utils/DeviceIdentifier.h"

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
        logger.logWithBorder("WiFi not connected, skipping update check");
        display.showSadFace();
        return;
    }
    
    String updateUrl = "/api/firmware/check?deviceId=" + DeviceIdentifier::getDeviceId() +
                      "&currentVersion=" + String(FIRMWARE_VERSION) +
                      "&modelType=" + String(MODEL_TYPE);
    
    logger.log("Update check URL: " + updateUrl);
    
    if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
        logger.logWithBorder("Failed to connect to update server");
        display.showSadFace();
        return;
    }
    
    String httpRequest =
        "GET " + updateUrl + " HTTP/1.1\r\n" +
        "Host: " + String(SERVER_URL) + "\r\n" +
        "Connection: close\r\n\r\n";
    
    wifiClient.print(httpRequest);
    logger.log("Sent update check request");
    
    // Read the entire response with better timeout and buffer handling
    String response = "";
    unsigned long timeout = millis();
    bool responseReceived = false;
    bool headersEnded = false;
    String jsonBody = "";
    
    while (millis() - timeout < API_TIMEOUT) {
        if (wifiClient.available()) {
            responseReceived = true;
            
            // Read the response line by line to better handle headers vs. body
            String line = wifiClient.readStringUntil('\n');
            
            // Check for HTTP status code
            if (line.startsWith("HTTP/1.")) {
                logger.log("Response status: " + line);
                if (line.indexOf("200 OK") < 0) {
                    logger.logWithBorder("Server returned non-200 status: " + line);
                    wifiClient.stop();
                    display.showSadFace();
                    return;
                }
            }
            
            // Detect end of headers
            if (line == "\r") {
                headersEnded = true;
                logger.log("Headers ended, reading body...");
                continue;
            }
            
            // If we're past headers, collect JSON body
            if (headersEnded) {
                jsonBody += line;
            } else {
                // Add important headers to log
                if (line.startsWith("Content-Type:") || 
                    line.startsWith("Content-Length:")) {
                    logger.log("Header: " + line);
                }
            }
        }
        
        // Check if connection closed and we have some response
        if (!wifiClient.connected() && responseReceived) {
            break;
        }
        
        delay(10);
    }
    
    wifiClient.stop();
    
    if (!responseReceived) {
        logger.logWithBorder("No response received from server");
        display.showSadFace();
        return;
    }
    
    if (!headersEnded || jsonBody.length() == 0) {
        logger.logWithBorder("Incomplete response received");
        logger.log("Headers ended: " + String(headersEnded));
        logger.log("Body length: " + String(jsonBody.length()));
        display.showSadFace();
        return;
    }
    
    // Clean up JSON body to ensure it's valid
    jsonBody.trim();
    
    // Find the actual JSON object if there's any wrapper text
    int firstBrace = jsonBody.indexOf('{');
    int lastBrace = jsonBody.lastIndexOf('}');
    
    if (firstBrace > -1 && lastBrace > -1) {
        jsonBody = jsonBody.substring(firstBrace, lastBrace + 1);
        
        if (handleUpdateResponse(jsonBody)) {
            return;
        }
    } else {
        logger.logWithBorder("Response does not contain valid JSON");
        logger.log("Response body: " + jsonBody);
    }
    
    logger.logWithBorder("Update check failed");
    display.showSadFace();
}

bool NetworkManager::handleUpdateResponse(String& jsonBody) {
    logger.log("Parsing update response: " + jsonBody);
    
    StaticJsonDocument<512> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, jsonBody);
    
    if (error) {
        logger.logWithBorder("JSON Parse Error: " + String(error.c_str()));
        return false;
    }
    
    // Check if JSON has the expected fields
    if (!jsonDoc.containsKey("updateAvailable")) {
        logger.logWithBorder("Invalid JSON response: missing 'updateAvailable' field");
        return false;
    }
    
    updateAvailable = jsonDoc["updateAvailable"].as<bool>();
    
    if (!updateAvailable) {
        String message = jsonDoc["message"] | "No updates available";
        logger.logWithBorder("Update Status: " + message);
        display.showHappyFace();
        return true;
    }
    
    // Check for required update fields
    if (!jsonDoc.containsKey("latestVersion")) {
        logger.logWithBorder("Invalid JSON: missing 'latestVersion' field");
        return false;
    }
    
    if (!jsonDoc.containsKey("size")) {
        logger.logWithBorder("Invalid JSON: missing 'size' field");
        return false;
    }
    
    latestFirmwareVersion = jsonDoc["latestVersion"].as<String>();
    logger.log("Latest firmware version: " + latestFirmwareVersion);
    logger.log("Current firmware version: " + String(FIRMWARE_VERSION));
    
    // Show update is available
    display.showUpdateAvailable();
    
    String downloadUrl = "/api/firmware/download?deviceId=" + DeviceIdentifier::getDeviceId() +
                        "&version=" + latestFirmwareVersion +
                        "&modelType=" + String(MODEL_TYPE);
    
    int firmwareSize = jsonDoc["size"] | 0;
    if (firmwareSize <= 0) {
        logger.logWithBorder("Invalid firmware size: " + String(firmwareSize));
        return false;
    }
    
    logger.log("Firmware size: " + String(firmwareSize) + " bytes");
    
    // Wait a moment to show that update is available before starting download
    delay(2000);
    
    return downloadAndApplyUpdate(downloadUrl, firmwareSize);
}

bool NetworkManager::downloadAndApplyUpdate(String& downloadUrl, int firmwareSize) {
    if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
        logger.logWithBorder("Failed to connect to download server");
        return false;
    }
    
    logger.logWithBorder("Downloading firmware update");
    logger.log("Update size: " + String(firmwareSize) + " bytes");
    logger.log("Download URL: " + downloadUrl);
    
    // Show update initialization animation
    display.showUpdateInitializing();
    
    String downloadRequest =
        "GET " + downloadUrl + " HTTP/1.1\r\n" +
        "Host: " + String(SERVER_URL) + "\r\n" +
        "Connection: close\r\n\r\n";
    
    wifiClient.print(downloadRequest);
    
    // Add timeout for header reading
    unsigned long headerTimeout = millis();
    bool headerFound = false;
    
    // Skip headers with timeout
    while (!headerFound && millis() - headerTimeout < API_TIMEOUT) {
        if (wifiClient.available()) {
            String line = wifiClient.readStringUntil('\n');
            logger.log("Header: " + line);
            if (line == "\r") {
                headerFound = true;
                break;
            }
        }
        delay(10);
    }
    
    if (!headerFound) {
        logger.logWithBorder("Failed to find header end marker");
        wifiClient.stop();
        return false;
    }
    
    // Initialize OTA update
    logger.log("Initializing OTA update storage");
    if (!OTAManager::beginUpdate(firmwareSize)) {
        logger.logWithBorder("Failed to initialize storage for update");
        wifiClient.stop();
        return false;
    }
    
    logger.log("Started firmware update process");
    int totalRead = 0;
    int lastProgressPercentage = -1; // Start with -1 to ensure first update is shown
    unsigned long downloadTimeout = millis();
    unsigned long lastProgressTime = millis();
    
    // Display initial update progress (0%)
    display.showUpdateProgress(0);
    
    // Check if any data is available initially
    if (!wifiClient.available()) {
        logger.log("Warning: No data available after headers");
        delay(1000); // Wait a bit for data to become available
    }
    
    // Main download loop with timeout handling
    while (totalRead < firmwareSize) {
        // Check for timeout conditions
        if (millis() - downloadTimeout > API_TIMEOUT * 3) {
            logger.logWithBorder("Download timeout - total timeout exceeded");
            OTAManager::abortUpdate();
            wifiClient.stop();
            return false;
        }
        
        // Check for stalled download
        if (millis() - lastProgressTime > 10000) { // 10 seconds without progress
            logger.logWithBorder("Download stalled - no progress for 10 seconds");
            OTAManager::abortUpdate();
            wifiClient.stop();
            return false;
        }
        
        // Handle data reading
        if (wifiClient.available()) {
            uint8_t buffer[128];
            int bytesRead = wifiClient.read(buffer, sizeof(buffer));
            
            if (bytesRead <= 0) {
                logger.log("Warning: Zero bytes read from client");
                delay(100);
                continue;
            }
            
            // Update the last progress time since we received data
            lastProgressTime = millis();
            
            // Write the data to flash storage
            int bytesWritten = OTAManager::write(buffer, bytesRead);
            if (bytesWritten != bytesRead) {
                logger.logWithBorder("Error writing firmware data: expected=" + 
                                  String(bytesRead) + ", actual=" + String(bytesWritten));
                OTAManager::abortUpdate();
                wifiClient.stop();
                return false;
            }
            
            totalRead += bytesRead;
            
            // Calculate progress percentage
            int progressPercentage = (totalRead * 100) / firmwareSize;
            
            // Update display only when percentage changes significantly
            if (progressPercentage / 5 > lastProgressPercentage / 5) {
                lastProgressPercentage = progressPercentage;
                display.showUpdateProgress(progressPercentage);
                logger.log("Downloaded: " + String(totalRead) + " bytes (" + String(progressPercentage) + "%)");
            }
        } else if (!wifiClient.connected()) {
            // Connection closed prematurely
            if (totalRead < firmwareSize) {
                logger.logWithBorder("Connection closed before download completed: " + 
                                 String(totalRead) + "/" + String(firmwareSize) + " bytes");
                OTAManager::abortUpdate();
                wifiClient.stop();
                return false;
            }
            break; // Download completed and connection closed normally
        } else {
            // No data available but still connected, wait briefly
            delay(10);
        }
    }
    
    // Close the WiFi client since we're done with it
    wifiClient.stop();
    
    // Check if download was successful
    if (totalRead < firmwareSize) {
        logger.logWithBorder("Download incomplete: " + String(totalRead) + 
                         "/" + String(firmwareSize) + " bytes received");
        OTAManager::abortUpdate();
        return false;
    }
    
    logger.log("Finalizing update");
    if (!OTAManager::endUpdate()) {
        logger.logWithBorder("Failed to finalize the update");
        return false;
    }
    
    logger.logWithBorder("Firmware downloaded successfully!");
    logger.log("Applying update...");
    
    // Show 100% update progress
    display.showUpdateProgress(100);
    
    // Notify server about the update
    StaticJsonDocument<256> statusDoc;
    statusDoc["deviceId"] = DeviceIdentifier::getDeviceId();
    statusDoc["firmwareVersion"] = FIRMWARE_VERSION;
    statusDoc["modelType"] = MODEL_TYPE;
    
    String statusData;
    serializeJson(statusDoc, statusData);
    logger.log("Sending update status to server");
    sendHttpPostRequest(statusData, "/api/device/register");
    
    // Give time to see the completion message before restart
    delay(3000);
    logger.logWithBorder("Restarting with new firmware");
    OTAManager::applyUpdate();  // This will restart the board
    return true;
} 