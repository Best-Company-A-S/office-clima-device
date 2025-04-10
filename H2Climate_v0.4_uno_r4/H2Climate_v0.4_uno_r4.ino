#include <ArduinoJson.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include "WiFiS3.h"
#include "DHT.h"
#include "secrets.h" // Contains WiFi credentials
#include "params.h" // Contains enviroment parameters
#include "Arduino_LED_Matrix.h" // For the built-in LED matrix

//¤=======================================================================================¤
//| TODO: Battery logging                                                                 |
//| TODO: Add sound sensor                                                                |                                                                 |
//| TODO: Changeable settings                                                             |
//| TODO: Use MAC address to make a unique DEVICE_ID                                      |
//| TODO: Add warning triggers at certain temperatures and humidities                     |
//| TODO: Store more sensor data before sending a packet to reduce packet spam            |
//¤=======================================================================================¤

DHT dht(DHT22_PIN, DHTTYPE);
WiFiClient wifiClient;
ArduinoLEDMatrix matrix;

// We'll use the built-in emoji frames for our status indicators
#define HAPPY_FACE LEDMATRIX_EMOJI_HAPPY
#define SAD_FACE LEDMATRIX_EMOJI_SAD
#define NEUTRAL_FACE LEDMATRIX_EMOJI_BASIC

// NTP Settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;
WiFiUDP ntpUDP;

unsigned long previousMillis = 0;

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

  // Register device with server
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["deviceId"] = DEVICE_ID;
  jsonDoc["modelType"] = MODEL_TYPE;
  jsonDoc["firmwareVersion"] = FIRMWARE_VERSION;

  String registerData;
  serializeJson(jsonDoc, registerData);

  sendHttpPostRequest(registerData, API_REGISTER_ROUTE);
  
  logToSerial("Setup complete");
}

// Helper functions for LED Matrix
void showHappyFace() {
  matrix.loadFrame(HAPPY_FACE);
}

void showSadFace() {
  matrix.loadFrame(SAD_FACE);
}

void showNeutralFace() {
  matrix.loadFrame(NEUTRAL_FACE);
}

// Add new animation function for retrying
void showRetryAnimation() {
  // Show a sequence of faces to indicate retrying
  for (int i = 0; i < RETRY_ANIMATION_BLINKS; i++) {
    showNeutralFace();
    delay(RETRY_ANIMATION_ON_TIME);
    matrix.clear();
    delay(RETRY_ANIMATION_OFF_TIME);
  }
}

//¤==============¤
//| Runtime Loop |
//¤==============¤========================================================================¤
void loop() {
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

  // Check if it's time to take a reading
  if (currentMillis - previousMillis > LOOP_INTERVAL) {
    previousMillis = currentMillis;
    
    logToSerial("Taking sensor readings");

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      logToSerial("Failed to read sensor");
      showSadFace();  // Show sad face for sensor failure
      return;
    }

    logToSerial("Temp: " + String(temperature) + "°C, Humidity: " + String(humidity) + "%");

    // Build JSON string for data packet
    StaticJsonDocument<256> jsonDoc;
    jsonDoc["deviceId"] = DEVICE_ID;
    jsonDoc["temperature"] = temperature;
    jsonDoc["humidity"] = humidity;
    jsonDoc["timestamp"] = now();

    String packetData;
    serializeJson(jsonDoc, packetData);

    sendHttpPostRequest(packetData, API_DATA_ROUTE);
    // Note: sendHttpPostRequest now handles all face displays internally
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
time_t getNtpTime() { // Retrieve the current time from the NTP server
  const int NTP_PACKET_SIZE = 48;
  byte packetBuffer[NTP_PACKET_SIZE];
  memset(packetBuffer, 0, NTP_PACKET_SIZE);

  // Initialize NTP request packet
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // rest of the packet is already 0

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
// Sends a HTTP POST request with a JSON string to a provided API endpoint
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

    // Try to connect to server
    showNeutralFace();
    int connectionAttempts = 0;
    bool connected = false;
    
    while (!connected && connectionAttempts < MAX_SERVER_CONNECT_ATTEMPTS) {
      if (wifiClient.connect(SERVER_URL, SERVER_PORT)) {
        connected = true;
      } else {
        connectionAttempts++;
        delay(RETRY_DELAY);
      }
    }
    
    if (!connected) {
      logToSerial("Failed to connect to server");
      apiAttempts++;
      
      // Show sad face if this was the last attempt
      if (apiAttempts >= MAX_API_ATTEMPTS) {
        logToSerial("Failed after " + String(MAX_API_ATTEMPTS) + " attempts");
        showSadFace();
        return false;
      }
      continue;
    }
   
    // Building HTTP POST request
    String httpRequest =
      "POST "            + apiRoute + " HTTP/1.1"       + "\r\n" +
      "Host: "           + String(SERVER_URL)           + "\r\n" +
      "Content-Type: "   + "application/json"           + "\r\n" +
      "Content-Length: " + String(jsonPayload.length()) + "\r\n" +
      "Connection: "     + "close"                      + "\r\n\r\n" +
      jsonPayload;
    
    wifiClient.print(httpRequest);
    
    // Wait for a response with a timeout defined by API_TIMEOUT
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
      
      // Show sad face if this was the last attempt
      if (apiAttempts >= MAX_API_ATTEMPTS) {
        logToSerial("Failed after " + String(MAX_API_ATTEMPTS) + " attempts");
        showSadFace();
        return false;
      }
      continue;
    }
    
    // Read API response from the server
    String response = "";
    while (wifiClient.available()) {
      char c = wifiClient.read();
      response += c;
    }
    
    // Close the connection to free resources
    wifiClient.stop();
    
    // Check response status code
    if (apiRoute == API_REGISTER_ROUTE) {
      // For registration, accept both 200/201 (new registration) and 409 (already registered)
      success = response.indexOf("200 OK") > 0 || 
                response.indexOf("201 Created") > 0 || 
                response.indexOf("409") > 0;
      
      if (success && response.indexOf("409") > 0) {
        logToSerial("Device already registered");
      } else if (success) {
        logToSerial("Device registered successfully");
      }
    } else {
      // For other endpoints, only accept 200/201
      success = response.indexOf("200 OK") > 0 || response.indexOf("201 Created") > 0;
      if (success) {
        logToSerial("Data sent successfully to " + apiRoute);
      }
    }

    if (!success) {
      logToSerial("Error in server response");
      apiAttempts++;
      
      // Show sad face if this was the last attempt
      if (apiAttempts >= MAX_API_ATTEMPTS) {
        logToSerial("Failed after " + String(MAX_API_ATTEMPTS) + " attempts");
        showSadFace();
        return false;
      }
    }
  }

  // Final status indication
  if (success) {
    showHappyFace();
    return true;
  } else {
    logToSerial("Failed after " + String(MAX_API_ATTEMPTS) + " attempts");
    showSadFace();
    return false;
  }
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
