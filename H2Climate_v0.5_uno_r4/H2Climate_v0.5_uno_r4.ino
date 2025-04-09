#include <ArduinoJson.h>
#include <Arduino.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <WiFiS3.h>
#include <DHT.h>
#include "secrets.h" // Contains WiFi credentials
#include "params.h" // Contains enviroment parameters
#include "FancyLog.h" // Header file for the FancyLog class

//¤=======================================================================================¤
//| TODO: Battery logging                                                                 |
//| TODO: Add sound sensor                                                                |
//| TODO: Changeable settings                                                             |
//| TODO: Use MAC address to make a unique DEVICE_ID                                      |
//| TODO: Add warning triggers at certain temperatures and humidities                     |
//| TODO: Store more sensor data before sending a packet to reduce packet spam            |
//¤=======================================================================================¤

// SMILY FACE STATUS TEST
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

#include "Arduino_LED_Matrix.h" // Built-in LED matrix

// Built-in emoji frames for status indicators
#define HAPPY_FACE LEDMATRIX_EMOJI_HAPPY
#define SAD_FACE LEDMATRIX_EMOJI_SAD
#define NEUTRAL_FACE LEDMATRIX_EMOJI_BASIC

ArduinoLEDMatrix matrix;

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

DHT dht(DHT22_PIN, DHTTYPE);
WiFiSSLClient wifiClient;

// NTP Settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;
WiFiUDP ntpUDP;

unsigned long previousMillis = 0;

// ### TEST ###
FancyLog fancyLog;
// ############

//¤================¤
//| Setup Function |
//¤================¤======================================================================¤
void setup() {
  Serial.begin(9600);
  fancyLog.toSerial("Starting H2Climate Device ID: " + String(DEVICE_ID), INFO);

// SMILY FACE STATUS TEST
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

  matrix.begin();
  showNeutralFace(); // Show neutral face during setup

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  // Initialize DHT-22 sensor
  dht.begin();

  // Attempt WiFi connection
  connectWiFi();

  // Initialize UDP socket for NTP
  ntpUDP.begin(5757); // Random local port

  // Sync time using NTP
  setSyncProvider(getNtpTime);
  if (timeStatus() == timeSet) {
    fancyLog.toSerial("Time synchronized", INFO);
  } else {
    fancyLog.toSerial("Failed to synchronize time", WARNING);
  }

// REGISTER PACKET TEST
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

  // Build JSON string for register packet
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["deviceId"] = DEVICE_ID;
  jsonDoc["modelType"] = MODEL_TYPE;
  jsonDoc["firmwareVersion"] = FIRMWARE_VERSION;

  String registerData;
  serializeJson(jsonDoc, registerData);

  sendHttpPostRequest(registerData, API_REGISTER_ROUTE);

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  fancyLog.toSerial("Setup complete, entering runtime loop", INFO);
}

//¤==============¤
//| Runtime Loop |
//¤==============¤========================================================================¤
void loop() {
  unsigned long currentMillis = millis();

  // Check if it's time to take a reading
  if (currentMillis - previousMillis > LOOP_INTERVAL) {
    previousMillis = currentMillis;

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      fancyLog.toSerial("Failed to read sensor", ERROR);
      //### SMILY FACE STATUS TEST ###
      showSadFace(); // Show sad face for sensor failure
      return;
    }

    fancyLog.toSerial("Temperature: " + String(temperature));
    fancyLog.toSerial("Humidity: " + String(humidity));

    // Build JSON string for data packet
    StaticJsonDocument<256> jsonDoc;
    jsonDoc["deviceId"] = DEVICE_ID;
    jsonDoc["temperature"] = temperature;
    jsonDoc["humidity"] = humidity;
    jsonDoc["timestamp"] = now();  // Time in seconds since epoch

    String packetData;
    serializeJson(jsonDoc, packetData);

    sendHttpPostRequest(packetData, API_DATA_ROUTE);
  }

  // Monitor WiFi connection and try to reconnect if lost
  if (WiFi.status() != WL_CONNECTED) {
    fancyLog.toSerial("WiFi disconnected. Attempting to reconnect...", WARNING);
    //### SMILY FACE STATUS TEST ###
    showSadFace(); // Show sad face if WiFi disconnected
    connectWiFi();
  }
}

// SMILY FACE STATUS TEST
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

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

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//¤==========================¤
//| WiFi Connection Function |
//¤==========================¤============================================================¤
void connectWiFi() {
  fancyLog.toSerial("Attempting WiFi connection to " + String(WIFI_SSID), INFO);
  //### SMILY FACE STATUS TEST ###
  showNeutralFace(); // Show neutral face while attempting to connect

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT) {
    Serial.print(".");
    delay(1000);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\n");
    fancyLog.toSerial("WiFi connected successfully");
    fancyLog.toSerial("Got IP: " + WiFi.localIP().toString());
    fancyLog.toSerial("Signal strength: " + String(WiFi.RSSI()));
    //### SMILY FACE STATUS TEST ###
    showHappyFace(); // Show happy face when connected
  } else {
    Serial.print("\n");
    fancyLog.toSerial("WiFi connection failed, will retry again later", WARNING);
    //### SMILY FACE STATUS TEST ###
    showSadFace(); // Show sad face if the connection failed
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
    fancyLog.toSerial("No NTP packet received", WARNING);
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
void sendHttpPostRequest(String jsonPayload, String apiRoute) {
  fancyLog.toSerial("Sending data to server...", INFO);
  fancyLog.toSerial(jsonPayload); // Log the JSON payload
  
  if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
    fancyLog.toSerial("Failed to connect to server", ERROR);
    //### SMILY FACE STATUS TEST ###
    showSadFace(); // Show sad face if the connection failed
    return;
  }
 
  // Building HTTP POST request
  // TODO: Refactor this bit into it's own function
  String httpRequest =
    "POST "            + apiRoute + " HTTP/1.1"       + "\r\n" +
    "Host: "           + String(SERVER_URL)           + "\r\n" +
    "Content-Type: "   + "application/json"           + "\r\n" +
    "Content-Length: " + String(jsonPayload.length()) + "\r\n" +
    "Connection: "     + "close"                  + "\r\n\r\n" +
    jsonPayload;
    
  Serial.println(httpRequest + "\n"); // Log the HTTP request
  wifiClient.print(httpRequest);
  
  // Wait for a response with a timeout defined by API_TIMEOUT
  unsigned long timeout = millis();
  while (wifiClient.available() == 0) {
    if (millis() - timeout > API_TIMEOUT) {
      fancyLog.toSerial("Server response timed out", ERROR);
      //### SMILY FACE STATUS TEST ###
      showSadFace(); // Show sad face if the response times out
      wifiClient.stop();
      return;
    }
  }
  
  // Read API response from the server
  String response = "";
  while (wifiClient.available()) {
    response += (char)wifiClient.read();
  }
  Serial.print("Response: " + response);

// REGISTER PACKET TEST
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

  bool success;

  if (apiRoute == API_REGISTER_ROUTE) {
      // For registration, accept both 200/201 (new registration) and 409 (already registered)
      success = response.indexOf("200 OK") > 0 ||
                response.indexOf("201 Created") > 0 ||
                response.indexOf("409") > 0;
      
      if (success && response.indexOf("409") > 0) {
        fancyLog.toSerial("Device already registered", INFO);
      } 
      else if (success) {
        fancyLog.toSerial("Device registered successfully", INFO);
      }

  } else {
    // For other endpoints, only accept 200/201
    success = response.indexOf("200 OK") > 0 || response.indexOf("201 Created") > 0;

    if (success) {
      fancyLog.toSerial("Data sent successfully to " + apiRoute, INFO);
      //### SMILY FACE STATUS TEST ###
      showHappyFace(); // Show happy face if data was sent successfully
    }
  }

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  // Close the connection to free resources
  wifiClient.stop();
}
