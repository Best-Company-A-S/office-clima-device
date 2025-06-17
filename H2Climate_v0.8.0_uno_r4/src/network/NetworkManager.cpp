#include "NetworkManager.h"

NetworkManager::NetworkManager(FancyLog& fancyLog, OTAManager& otaManager, DisplayManager& display)
    : fancyLog(fancyLog), otaManager(otaManager), display(display), updateAvailable(false) {}

//#####################################################################################################################

void NetworkManager::begin() {
    // Connect to WiFi
    connectWiFi();
    
    // Initialize OTA
    OTAManager::begin(WiFi.localIP(), WIFI_SSID, WIFI_PASS);
}

//#####################################################################################################################

void NetworkManager::pollOTA() {
    OTAManager::poll();
}

//#####################################################################################################################

bool NetworkManager::connectWiFi() {
    fancyLog.toSerial("Connecting to WiFi: " + String(WIFI_SSID), INFO);
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
		fancyLog.toSerial("WiFi connected successfully | IP: " + WiFi.localIP().toString() +
						  " | RSSI: " + String(WiFi.RSSI()) + " dBm", INFO);
        display.showHappyFace();
        return true;
    } else {
        fancyLog.toSerial("WiFi connection failed", ERROR);
        display.showSadFace();
        return false;
    }
}

//#####################################################################################################################

bool NetworkManager::sendHttpPostRequest(String jsonPayload, String apiRoute) {
    int apiAttempts = 0;
    bool success = false;
    
    while (apiAttempts < MAX_API_ATTEMPTS && !success) {
        if (apiAttempts > 0) {
            fancyLog.toSerial("Retry attempt " + String(apiAttempts) + " of " + String(MAX_API_ATTEMPTS - 1), INFO);
            display.showRetryAnimation();
        }
        
        if (!isConnected()) {
            fancyLog.toSerial("WiFi not connected. Reconnecting...", WARNING);
            display.showSadFace();
            if (!connectWiFi()) {
                return false;
            }
        }
        
        display.showNeutralFace();
        if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
            fancyLog.toSerial("Failed to connect to server", ERROR);
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
                    fancyLog.toSerial("Data sent successfully to " + apiRoute);
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

//#####################################################################################################################

void NetworkManager::checkForUpdates() {
    fancyLog.toSerial("Checking for firmware updates...", INFO);
    fancyLog.toSerial("Current version: " + String(FIRMWARE_VERSION), INFO);
    display.showNeutralFace();
    
    if (!isConnected()) {
        fancyLog.toSerial("WiFi not connected, skipping update check", WARNING);
        display.showSadFace();
        return;
    }
    
    String updateUrl = "/api/firmware/check?deviceId=" + DeviceIdentifier::getDeviceId() +
                      "&currentVersion=" + String(FIRMWARE_VERSION) +
                      "&modelType=" + String(MODEL_TYPE);
    
    fancyLog.toSerial("Update check URL: " + updateUrl);
    
    if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
        fancyLog.toSerial("Failed to connect to update server", ERROR);
        display.showSadFace();
        return;
    }
    
    String httpRequest =
        "GET " + updateUrl + " HTTP/1.1\r\n" +
        "Host: " + String(SERVER_URL) + "\r\n" +
        "Connection: close\r\n\r\n";
    
    wifiClient.print(httpRequest);
    fancyLog.toSerial("Sent update check request", INFO);
    
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
                fancyLog.toSerial("Response status: " + line);
                if (line.indexOf("200 OK") < 0) {
                    fancyLog.toSerial("Server returned non-200 status: " + line);
                    wifiClient.stop();
                    display.showSadFace();
                    return;
                }
            }
            
            // Detect end of headers
            if (line == "\r") {
                headersEnded = true;
                fancyLog.toSerial("Headers ended, reading body...", INFO);
                continue;
            }
            
            // If we're past headers, collect JSON body
            if (headersEnded) {
                jsonBody += line;
            } else {
                // Add important headers to log
                if (line.startsWith("Content-Type:") || 
                    line.startsWith("Content-Length:")) {
                    fancyLog.toSerial("Header: " + line);
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
        fancyLog.toSerial("No response received from server", ERROR);
        display.showSadFace();
        return;
    }
    
    if (!headersEnded || jsonBody.length() == 0) {
        fancyLog.toSerial("Incomplete response received", WARNING);
        fancyLog.toSerial("Headers ended: " + String(headersEnded));
        fancyLog.toSerial("Body length: " + String(jsonBody.length()));
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
        fancyLog.toSerial("Response does not contain valid JSON", WARNING);
        fancyLog.toSerial("Response body: " + jsonBody);
    }
    
    fancyLog.toSerial("Update check failed", ERROR);
    display.showSadFace();
}

//#####################################################################################################################

bool NetworkManager::handleUpdateResponse(String& jsonBody) {
    fancyLog.toSerial("Parsing update response: " + jsonBody);
    
    StaticJsonDocument<512> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, jsonBody);
    
    if (error) {
        fancyLog.toSerial("JSON Parse Error: " + String(error.c_str()));
        return false;
    }
    
    // Check if JSON has the expected fields
    if (!jsonDoc.containsKey("updateAvailable")) {
        fancyLog.toSerial("Invalid JSON response: missing 'updateAvailable' field", WARNING);
        return false;
    }
    
    updateAvailable = jsonDoc["updateAvailable"].as<bool>();
    
    if (!updateAvailable) {
        String message = jsonDoc["message"] | "No updates available";
        fancyLog.toSerial("Update Status: " + message);
        display.showHappyFace();
        return true;
    }
    
    // Check for required update fields
    if (!jsonDoc.containsKey("latestVersion")) {
        fancyLog.toSerial("Invalid JSON: missing 'latestVersion' field", WARNING);
        return false;
    }
    
    if (!jsonDoc.containsKey("size")) {
        fancyLog.toSerial("Invalid JSON: missing 'size' field", WARNING);
        return false;
    }
    
    latestFirmwareVersion = jsonDoc["latestVersion"].as<String>();
    fancyLog.toSerial("Latest firmware version: " + latestFirmwareVersion, INFO);
    fancyLog.toSerial("Current firmware version: " + String(FIRMWARE_VERSION), INFO);
    
    // Show update is available
    display.showUpdateAvailable();
    
    String downloadUrl = "/api/firmware/download?deviceId=" + DeviceIdentifier::getDeviceId() +
                        "&version=" + latestFirmwareVersion +
                        "&modelType=" + String(MODEL_TYPE);
    
    int firmwareSize = jsonDoc["size"] | 0;
    if (firmwareSize <= 0) {
        fancyLog.toSerial("Invalid firmware size: " + String(firmwareSize), WARNING);
        return false;
    }
    
    fancyLog.toSerial("Firmware size: " + String(firmwareSize) + " bytes", INFO);
    
    // Wait a moment to show that update is available before starting download
    delay(2000);
    
    return downloadAndApplyUpdate(downloadUrl, firmwareSize);
}

//#####################################################################################################################

bool NetworkManager::downloadAndApplyUpdate(String& downloadUrl, int firmwareSize) {
    if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
        fancyLog.toSerial("Failed to connect to download server", ERROR);
        return false;
    }
    
    fancyLog.toSerial("Downloading firmware update", INFO);
    fancyLog.toSerial("Update size: " + String(firmwareSize) + " bytes");
    fancyLog.toSerial("Download URL: " + downloadUrl);
    
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
            fancyLog.toSerial("Header: " + line);
            if (line == "\r") {
                headerFound = true;
                break;
            }
        }
        delay(10);
    }
    
    if (!headerFound) {
        fancyLog.toSerial("Failed to find header end marker", ERROR);
        wifiClient.stop();
        return false;
    }
    
    // Initialize OTA update
    fancyLog.toSerial("Initializing OTA update storage", INFO);
    if (!OTAManager::beginUpdate(firmwareSize)) {
        fancyLog.toSerial("Failed to initialize storage for update", ERROR);
        wifiClient.stop();
        return false;
    }
    
    fancyLog.toSerial("Started firmware update process", INFO);
    int totalRead = 0;
    int lastProgressPercentage = -1; // Start with -1 to ensure first update is shown
    unsigned long downloadTimeout = millis();
    unsigned long lastProgressTime = millis();
    
    // Display initial update progress (0%)
    display.showUpdateProgress(0);
    
    // Check if any data is available initially
    if (!wifiClient.available()) {
        fancyLog.toSerial("No data available after headers", WARNING);
        delay(1000); // Wait a bit for data to become available
    }
    
    // Main download loop with timeout handling
    while (totalRead < firmwareSize) {
        // Check for timeout conditions
        if (millis() - downloadTimeout > API_TIMEOUT * 3) {
            fancyLog.toSerial("Download timeout - total timeout exceeded", WARNING);
            OTAManager::abortUpdate();
            wifiClient.stop();
            return false;
        }
        
        // Check for stalled download
        if (millis() - lastProgressTime > 10000) { // 10 seconds without progress
            fancyLog.toSerial("Download stalled - no progress for 10 seconds",  WARNING);
            OTAManager::abortUpdate();
            wifiClient.stop();
            return false;
        }
        
        // Handle data reading
        if (wifiClient.available()) {
            uint8_t buffer[128];
            int bytesRead = wifiClient.read(buffer, sizeof(buffer));
            
            if (bytesRead <= 0) {
                fancyLog.toSerial("Zero bytes read from client", WARNING);
                delay(100);
                continue;
            }
            
            // Update the last progress time since we received data
            lastProgressTime = millis();
            
            // Write the data to flash storage
            int bytesWritten = OTAManager::write(buffer, bytesRead);
            if (bytesWritten != bytesRead) {
                fancyLog.toSerial("Error writing firmware data: expected=" +
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
                fancyLog.toSerial("Downloaded: " + String(totalRead) + " bytes (" + String(progressPercentage) + "%)");
            }
        } else if (!wifiClient.connected()) {
            // Connection closed prematurely
            if (totalRead < firmwareSize) {
                fancyLog.toSerial("Connection closed before download completed: " +
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
        fancyLog.toSerial("Download incomplete: " + String(totalRead) +
                         "/" + String(firmwareSize) + " bytes received");
        OTAManager::abortUpdate();
        return false;
    }
    
    fancyLog.toSerial("Finalizing update", INFO);
    if (!OTAManager::endUpdate()) {
        fancyLog.toSerial("Failed to finalize the update", ERROR);
        return false;
    }
    
    fancyLog.toSerial("Firmware downloaded successfully!", INFO);
    fancyLog.toSerial("Applying update...", INFO);
    
    // Show 100% update progress
    display.showUpdateProgress(100);
    
    // Notify server about the update
    StaticJsonDocument<256> statusDoc;
    statusDoc["deviceId"] = DeviceIdentifier::getDeviceId();
    statusDoc["firmwareVersion"] = FIRMWARE_VERSION;
    statusDoc["modelType"] = MODEL_TYPE;
    
    String statusData;
    serializeJson(statusDoc, statusData);
    fancyLog.toSerial("Sending update status to server", INFO);
    sendHttpPostRequest(statusData, "/api/device/register");
    
    // Give time to see the completion message before restart
    delay(3000);
    fancyLog.toSerial("Restarting with new firmware", INFO);
    OTAManager::applyUpdate();  // This will restart the board
    return true;
} 