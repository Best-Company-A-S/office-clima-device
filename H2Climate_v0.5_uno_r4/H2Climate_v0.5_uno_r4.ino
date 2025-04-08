#include <ArduinoJson.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include "WiFiS3.h"
#include "DHT.h"
#include "secrets.h" // Contains WiFi credentials
#include "params.h" // Contains enviroment parameters
#include "FancyLog.h" // FancyLog class

//¤=======================================================================================¤
//| TODO: Battery logging                                                                 |
//| TODO: Add sound sensor                                                                |
//| TODO: Changeable settings                                                             |
//| TODO: Use MAC address to make a unique DEVICE_ID                                      |
//| TODO: Add warning triggers at certain temperatures and humidities                     |
//| TODO: Store more sensor data before sending a packet to reduce packet spam            |
//¤=======================================================================================¤

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
  dht.begin(); // Initialize DHT-22 sensor

  // Attempt WiFi connection
  connectWiFi();

  // Initialize UDP socket for NTP
  ntpUDP.begin(5757); // Random local port

  // Sync time using NTP
  setSyncProvider(getNtpTime);
  if (timeStatus() == timeSet) {
    fancyLog.logToSerial("Time synchronized");
  } else {
    fancyLog.logToSerial("Failed to synchronize time");
  }



  // ### TEST ###
  fancyLog.logToSerial("FancyLog test 1");
  fancyLog.logToSerial("FancyLog test 2", INFO);
  fancyLog.logToSerial("FancyLog test 3", WARNING);
  fancyLog.logToSerial("FancyLog test 4", ERROR);
  // ############

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
      fancyLog.logToSerial("Failed to read sensor");
      return;
    }

    fancyLog.logToSerial("Temperature: " + String(temperature));
    fancyLog.logToSerial("Humidity: " + String(humidity));

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
    fancyLog.logToSerial("WiFi disconnected. Attempting to reconnect...");
    connectWiFi();
  }
}

//¤==========================¤
//| WiFi Connection Function |
//¤==========================¤============================================================¤
void connectWiFi() {
  fancyLog.logToSerial("Attempting WiFi connection to " + String(WIFI_SSID));
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT) {
    Serial.print(".");
    delay(1000);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    fancyLog.logToSerial("WiFi connected successfully");
    fancyLog.logToSerial("Got IP: " + WiFi.localIP().toString());
    fancyLog.logToSerial("Signal strength: " + String(WiFi.RSSI()));
  } else {
    fancyLog.logToSerial("WiFi connection failed, will retry later");
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
    fancyLog.logToSerial("No NTP packet received");
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
  fancyLog.logToSerial("Sending data to server...");
  fancyLog.logToSerial(jsonPayload); // Log the JSON payload
  
  if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
    fancyLog.logToSerial("Error: Failed to connect to server");
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
      fancyLog.logToSerial("Error: Server response timed out.");
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
        fancyLog.logToSerial("Device already registered");
      } 
      else if (success) {
        fancyLog.logToSerial("Device registered successfully");
      }

  } else {
    // For other endpoints, only accept 200/201
    success = response.indexOf("200 OK") > 0 || response.indexOf("201 Created") > 0;

    if (success) {
      fancyLog.logToSerial("Data sent successfully to " + apiRoute);
    }
  }

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  // Close the connection to free resources
  wifiClient.stop();
}
