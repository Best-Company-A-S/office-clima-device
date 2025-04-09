#include <ArduinoJson.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include "WiFiS3.h"
#include "DHT.h"
#include "secrets.h" // Contains WiFi credentials
#include "params.h" // Contains environment parameters
#include "Arduino_LED_Matrix.h" // For the built-in LED matrix
#include <ArduinoOTA.h> // Add ArduinoOTA support

//¤=======================================================================================¤
//| TODO: Battery logging                                                                 |
//| TODO: Add sound sensor                                                               |
//| TODO: Changeable settings                                                            |
//| TODO: Use MAC address to make a unique DEVICE_ID                                     |
//| TODO: Add warning triggers at certain temperatures and humidities                    |
//| TODO: Store more sensor data before sending a packet to reduce packet spam           |
//¤=======================================================================================¤

// Forward declarations of functions
void showHappyFace();
void showSadFace();
void showNeutralFace();
void showRetryAnimation();
void logToSerial(String logMessage);
bool sendHttpPostRequest(String jsonPayload, String apiRoute);

// Global variables
DHT dht(DHT22_PIN, DHTTYPE);
WiFiClient wifiClient;
ArduinoLEDMatrix matrix;

// NTP Settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;
WiFiUDP ntpUDP;

// Timing variables
unsigned long previousMillis = 0;
unsigned long previousUpdateCheckMillis = 0;

// Update-related variables
String latestFirmwareVersion = "";
bool updateAvailable = false;
bool autoUpdateTriggered = false;

// Define update check interval if not in params.h
#ifndef CHECK_INTERVAL
#define CHECK_INTERVAL 3600000UL  // 1 hour in milliseconds
#endif

// We'll use the built-in emoji frames for our status indicators
#define HAPPY_FACE LEDMATRIX_EMOJI_HAPPY
#define SAD_FACE LEDMATRIX_EMOJI_SAD
#define NEUTRAL_FACE LEDMATRIX_EMOJI_BASIC

//¤================¤
//| Setup Function |
//¤================¤======================================================================¤
void setup() {
  Serial.begin(9600);
  matrix.begin();
  showNeutralFace(); // Show neutral face during setup
  
  logToSerial("Starting H2Climate Device");
  logToSerial("Device ID: " + String(DEVICE_ID));
  
  dht.begin(); // Initialize DHT-22 sensor

  // Attempt WiFi connection
  connectWiFi();

  // Initialize UDP socket for NTP
  ntpUDP.begin(5757); // Random local port

  // Sync time using NTP
  setSyncProvider(getNtpTime);
  if (timeStatus() == timeSet) {
    logToSerial("Time synchronized");
  } else {
    logToSerial("Failed to synchronize time");
  }

  // Initialize ArduinoOTA with basic configuration
  ArduinoOTA.begin(WiFi.localIP(), WIFI_SSID, WIFI_PASS, InternalStorage);
  
  // Register device with server
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["deviceId"] = DEVICE_ID;
  jsonDoc["modelType"] = MODEL_TYPE;
  jsonDoc["firmwareVersion"] = FIRMWARE_VERSION;

  String registerData;
  serializeJson(jsonDoc, registerData);

  sendHttpPostRequest(registerData, API_REGISTER_ROUTE);
  
  // Check for updates on startup
  checkForUpdates();
  
  logToSerial("Setup complete");
}

//¤==============¤
//| Runtime Loop |
//¤==============¤========================================================================¤
void loop() {
  // Handle potential OTA updates from Arduino IDE
  ArduinoOTA.poll();

  unsigned long currentMillis = millis();
  unsigned long timeUntilNextReading = 0;
  
  // Check WiFi status and update face accordingly
  if (WiFi.status() != WL_CONNECTED) {
    showSadFace();
    logToSerial("WiFi disconnected. Reconnecting...");
    connectWiFi();
  }
  
  // Calculate time until next reading
  if (currentMillis - previousMillis < LOOP_INTERVAL) {
    timeUntilNextReading = LOOP_INTERVAL - (currentMillis - previousMillis);
    
    // Every 10 seconds, show time remaining until next data transmission
    if (timeUntilNextReading % 10000 < 100) {
      Serial.println("Next transmission in " + String(timeUntilNextReading / 1000) + "s");
    }
  }

  // Check for firmware updates periodically using your API
  if (currentMillis - previousUpdateCheckMillis > CHECK_INTERVAL) {
    previousUpdateCheckMillis = currentMillis;
    checkForUpdates();
  }

  // If update is available, show indicator periodically
  if (updateAvailable && currentMillis % 30000 < 1000) { // Flash every 30 seconds
    showUpdateAvailable();
    showNeutralFace();
  }
}

//¤=========================¤
//| LED Matrix Functions    |
//¤=========================¤===============================================================¤
void showHappyFace() {
  matrix.loadFrame(HAPPY_FACE);
}

void showSadFace() {
  matrix.loadFrame(SAD_FACE);
}

void showNeutralFace() {
  matrix.loadFrame(NEUTRAL_FACE);
}

void showRetryAnimation() {
  // Show a sequence of faces to indicate retrying
  for (int i = 0; i < RETRY_ANIMATION_BLINKS; i++) {
    showNeutralFace();
    delay(RETRY_ANIMATION_ON_TIME);
    matrix.clear();
    delay(RETRY_ANIMATION_OFF_TIME);
  }
}

void showUpdateAvailable() {
  // Custom animation for update available
  for (int i = 0; i < 3; i++) {
    showHappyFace();
    delay(500);
    showNeutralFace();
    delay(500);
  }
}

//¤==========================¤
//| WiFi Connection Function |
//¤==========================¤============================================================¤
void connectWiFi() {
  logToSerial("Connecting to WiFi: " + String(WIFI_SSID));
  showNeutralFace(); // Show neutral face while attempting to connect
  
  // Disconnect if already connected
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
    delay(1000);
  }
  
  // Try to connect
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  unsigned long startAttemptTime = millis();
  int attempts = 0;
  
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT) {
    Serial.print(".");
    delay(1000);
    
    // If taking too long, try reconnecting every 5 seconds
    if (attempts++ > 5) {
      WiFi.disconnect();
      delay(500);
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      attempts = 0;
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    logToSerial("WiFi connected, IP: " + WiFi.localIP().toString());
    showHappyFace(); // Show happy face when connected
  } else {
    logToSerial("WiFi connection failed");
    showSadFace(); // Show sad face when connection fails
  }
}

//¤==============================¤
//| NTP Synchronization Function |
//¤==============================¤========================================================¤
time_t getNtpTime() {
  const int NTP_PACKET_SIZE = 48;
  byte packetBuffer[NTP_PACKET_SIZE];
  memset(packetBuffer, 0, NTP_PACKET_SIZE);

  // Initialize NTP request packet
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision

  ntpUDP.beginPacket(ntpServer, 123);
  ntpUDP.write(packetBuffer, NTP_PACKET_SIZE);
  ntpUDP.endPacket();

  delay(1000); // wait for response

  int cb = ntpUDP.parsePacket();
  if (!cb) {
    logToSerial("No NTP packet received");
    return 0;
  } else {
    ntpUDP.read(packetBuffer, NTP_PACKET_SIZE);
    unsigned long secsSince1900 =  (unsigned long)packetBuffer[40] << 24 |
                                   (unsigned long)packetBuffer[41] << 16 |
                                   (unsigned long)packetBuffer[42] << 8  |
                                   (unsigned long)packetBuffer[43];
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears;
    return epoch + gmtOffset_sec + daylightOffset_sec;
  }
}

//¤============================¤
//| HTTP POST Request Function |
//¤============================¤==========================================================¤
bool sendHttpPostRequest(String jsonPayload, String apiRoute) {
  int apiAttempts = 0;
  bool success = false;

  while (apiAttempts < MAX_API_ATTEMPTS && !success) {
    if (apiAttempts > 0) {
      logToSerial("Retry attempt " + String(apiAttempts) + " of " + String(MAX_API_ATTEMPTS - 1));
      showRetryAnimation();
    }

    if (WiFi.status() != WL_CONNECTED) {
      logToSerial("WiFi not connected. Reconnecting...");
      showSadFace();
      connectWiFi();
      if (WiFi.status() != WL_CONNECTED) {
        logToSerial("Failed to reconnect WiFi");
        showSadFace();
        return false;
      }
    }

    showNeutralFace();
    if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
      logToSerial("Failed to connect to server");
      apiAttempts++;
      if (apiAttempts >= MAX_API_ATTEMPTS) {
        showSadFace();
        return false;
      }
      continue;
    }
   
    String httpRequest =
      "POST "            + apiRoute + " HTTP/1.1"       + "\r\n" +
      "Host: "           + String(SERVER_URL)           + "\r\n" +
      "Content-Type: "   + "application/json"           + "\r\n" +
      "Content-Length: " + String(jsonPayload.length()) + "\r\n" +
      "Connection: "     + "close"                      + "\r\n\r\n" +
      jsonPayload;
    
    wifiClient.print(httpRequest);
    
    unsigned long timeout = millis();
    bool responseReceived = false;
    
    while (millis() - timeout < API_TIMEOUT) {
      if (wifiClient.available()) {
        responseReceived = true;
        break;
      }
      delay(10);
    }

    if (!responseReceived) {
      logToSerial("Server response timed out");
      wifiClient.stop();
      apiAttempts++;
      if (apiAttempts >= MAX_API_ATTEMPTS) {
        showSadFace();
        return false;
      }
      continue;
    }
    
    String response = "";
    while (wifiClient.available()) {
      response += (char)wifiClient.read();
    }
    
    wifiClient.stop();
    
    if (apiRoute == API_REGISTER_ROUTE) {
      success = response.indexOf("200 OK") > 0 || 
                response.indexOf("201 Created") > 0 || 
                response.indexOf("409") > 0;
      
      if (success && response.indexOf("409") > 0) {
        logToSerial("Device already registered");
      } else if (success) {
        logToSerial("Device registered successfully");
      }
    } else {
      success = response.indexOf("200 OK") > 0 || response.indexOf("201 Created") > 0;
      if (success) {
        logToSerial("Data sent successfully to " + apiRoute);
      }
    }

    if (!success) {
      logToSerial("Error in server response");
      apiAttempts++;
      if (apiAttempts >= MAX_API_ATTEMPTS) {
        showSadFace();
        return false;
      }
    }
  }

  if (success) {
    showHappyFace();
    return true;
  } else {
    showSadFace();
    return false;
  }
}

//¤===========================¤
//| Check for Updates Function |
//¤===========================¤===========================================================¤
void checkForUpdates() {
  logToSerial("Checking for firmware updates...");
  logToSerial("Current version: " + String(FIRMWARE_VERSION));
  showNeutralFace();
  
  if (WiFi.status() != WL_CONNECTED) {
    logToSerial("WiFi not connected, skipping update check");
    showSadFace();
    return;
  }

  String updateUrl = "/api/firmware/check?deviceId=" + String(DEVICE_ID) + 
                    "&currentVersion=" + String(FIRMWARE_VERSION) + 
                    "&modelType=" + String(MODEL_TYPE);
  
  logToSerial("=== UPDATE CHECK REQUEST ===");
  logToSerial("Target: " + String(SERVER_URL) + ":" + String(SERVER_PORT));
  logToSerial("Method: GET");
  logToSerial("Path: " + updateUrl);
  
  if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
    logToSerial("Failed to connect to update server");
    showSadFace();
    return;
  }
  
  String httpRequest =
    "GET "             + updateUrl + " HTTP/1.1"      + "\r\n" +
    "Host: "           + String(SERVER_URL)           + "\r\n" +
    "Connection: "     + "close"                      + "\r\n\r\n";
  
  wifiClient.print(httpRequest);
  
  // Wait for response
  unsigned long timeout = millis();
  while (wifiClient.available() == 0) {
    if (millis() - timeout > API_TIMEOUT) {
      logToSerial("Update check timed out");
      wifiClient.stop();
      showSadFace();
      return;
    }
  }

  // Read the entire response first
  String fullResponse = "";
  timeout = millis();
  
  while (millis() - timeout < API_TIMEOUT) {
    while (wifiClient.available()) {
      char c = wifiClient.read();
      fullResponse += c;
    }
    
    if (!wifiClient.connected()) {
      break;
    }
    
    delay(10);
  }
  
  wifiClient.stop();
  
  logToSerial("=== FULL RESPONSE ===");
  logToSerial(fullResponse);
  
  // Find the start of JSON data (after headers)
  int jsonStart = fullResponse.indexOf("\r\n\r\n");
  if (jsonStart == -1) {
    logToSerial("No JSON data found in response");
    return;
  }
  
  // Extract just the JSON part and clean it
  String jsonBody = fullResponse.substring(jsonStart + 4);
  
  // Find the first '{' and last '}'
  int firstBrace = jsonBody.indexOf('{');
  int lastBrace = jsonBody.lastIndexOf('}');
  
  if (firstBrace == -1 || lastBrace == -1) {
    logToSerial("Invalid JSON format - missing braces");
    return;
  }
  
  // Extract only the JSON object
  jsonBody = jsonBody.substring(firstBrace, lastBrace + 1);
  jsonBody.trim();
  
  logToSerial("=== CLEANED JSON BODY ===");
  logToSerial(jsonBody);
  
  // Try parsing the JSON
  StaticJsonDocument<512> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, jsonBody);
  
  if (error) {
    logToSerial("JSON Parse Error: " + String(error.c_str()));
    logToSerial("JSON Body Length: " + String(jsonBody.length()));
    logToSerial("First 50 chars: " + jsonBody.substring(0, 50));
    showSadFace();
    return;
  }
  
  // Extract update information
  updateAvailable = jsonDoc["updateAvailable"].as<bool>();
  
  if (!updateAvailable) {
    String message = jsonDoc["message"] | "No updates available";
    logToSerial(message);
    showHappyFace();
    return;
  }
  
  latestFirmwareVersion = jsonDoc["latestVersion"].as<String>();
  String releaseNotes = jsonDoc["releaseNotes"] | "No release notes provided";
  int firmwareSize = jsonDoc["size"] | 0;
  
  logToSerial("=== UPDATE AVAILABLE ===");
  logToSerial("Current version: " + String(FIRMWARE_VERSION));
  logToSerial("Latest version: " + latestFirmwareVersion);
  logToSerial("Release notes: " + releaseNotes);
  logToSerial("Firmware size: " + String(firmwareSize) + " bytes");
  
  // Use the correct download API endpoint
  String downloadUrl = "/api/firmware/download?deviceId=" + String(DEVICE_ID) + 
                      "&version=" + latestFirmwareVersion +
                      "&modelType=" + String(MODEL_TYPE);
  
  logToSerial("Starting firmware download...");
  logToSerial("Download URL: " + downloadUrl);
  
  // Connect to server for download
  if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
    logToSerial("Failed to connect to download server");
    return;
  }
  
  // Request the firmware file
  String downloadRequest = 
    "GET " + downloadUrl + " HTTP/1.1\r\n" +
    "Host: " + String(SERVER_URL) + "\r\n" +
    "Connection: close\r\n\r\n";
  
  logToSerial("Sending download request...");
  wifiClient.print(downloadRequest);
  
  // Wait for response
  timeout = millis();
  while (wifiClient.available() == 0) {
    if (millis() - timeout > API_TIMEOUT) {
      logToSerial("Firmware download timed out");
      wifiClient.stop();
      return;
    }
  }
  
  // Read and log the headers
  logToSerial("=== DOWNLOAD RESPONSE HEADERS ===");
  String contentLength = "";
  while (wifiClient.available()) {
    String line = wifiClient.readStringUntil('\n');
    logToSerial(line);
    if (line.startsWith("Content-Length: ")) {
      contentLength = line.substring(16);
      // Verify the content length matches expected firmware size
      if (contentLength.toInt() != firmwareSize) {
        logToSerial("Warning: Content length (" + contentLength + ") differs from expected size (" + String(firmwareSize) + ")");
      }
    }
    if (line == "\r") {
      break;
    }
  }

  if (firmwareSize <= 0) {
    logToSerial("Invalid firmware size");
    wifiClient.stop();
    return;
  }
  
  // Start the OTA update
  if (InternalStorage.open(firmwareSize)) {
    logToSerial("Started firmware update process");
    logToSerial("Expected size: " + String(firmwareSize) + " bytes");
    
    // Read the firmware byte by byte
    int totalRead = 0;
    
    while (wifiClient.available() && totalRead < firmwareSize) {
      uint8_t b = wifiClient.read();
      if (InternalStorage.write(b) != 1) {
        logToSerial("Error writing to internal storage");
        wifiClient.stop();
        return;
      }
      totalRead++;
      
      if (totalRead % 1024 == 0) {
        logToSerial("Downloaded: " + String(totalRead) + " bytes");
        logToSerial("Progress: " + String((totalRead * 100) / firmwareSize) + "%");
      }
    }
    
    InternalStorage.close(); // No need to check return value, it's void
    
    if (totalRead == firmwareSize) {
      logToSerial("Firmware downloaded successfully!");
      logToSerial("Total bytes: " + String(totalRead));
      logToSerial("Applying update...");
      
      // Notify server about the update
      StaticJsonDocument<256> statusDoc;
      statusDoc["deviceId"] = DEVICE_ID;
      statusDoc["firmwareStatus"] = "UPDATING";
      statusDoc["currentVersion"] = FIRMWARE_VERSION;
      statusDoc["targetVersion"] = latestFirmwareVersion;
      
      String statusData;
      serializeJson(statusDoc, statusData);
      sendHttpPostRequest(statusData, "/api/devices/" + String(DEVICE_ID));
      
      delay(1000); // Give time for the status to be sent
      
      logToSerial("Restarting device to apply update...");
      InternalStorage.apply(); // This will restart the device
    } else {
      logToSerial("Firmware download incomplete!");
      logToSerial("Expected: " + String(firmwareSize) + " bytes");
      logToSerial("Received: " + String(totalRead) + " bytes");
    }
  } else {
    logToSerial("Could not open internal storage for writing");
  }
  
  wifiClient.stop();
}

//¤========================¤
//| Fancy Logging Function |
//¤========================¤==============================================================¤
void logToSerial(String logMessage) {
  int messageLength = logMessage.length();
  String messageBorder = "";

  // Make a string of '=' characters matching the length of logMessage
  for (int i = 0; i < messageLength; i++) {
    messageBorder += "=";
  }

  Serial.println("¤" + messageBorder + "¤");
  Serial.println("|" + logMessage + "|");
  Serial.println("¤" + messageBorder + "¤");
}