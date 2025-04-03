#include "WiFiS3.h"
#include "ArduinoHttpClient.h"
#include "DHT.h"
#include "secrets.h" // Contains WiFi credentials
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <WiFiUdp.h>

//#########################################################################################

// TODO: Battery logging
// TODO: Changeable settings
// TODO: Use MAC address to make a unique DEVICE_ID
// TODO: Add warning triggers at certain temperatures and humidities
// TODO: Store more sensor data before sending a packet to reduce packet spam

//#########################################################################################

// Device details
#define DEVICE_ID "6fe26f8eaf7e" // Test ID
#define MODEL_TYPE "Arduino_UNO_R4_WiFi"
#define FIRMWARE_VERSION "v0.3"

// Sensor details
#define DHT22_PIN 12
#define DHTTYPE DHT22

// Server details
#define SERVER_URL "clima-app-blush-beta.vercel.app"
#define SERVER_PORT 443
#define API_ROUTE "/api/device/test"

// WiFi timeout settings
#define WIFI_TIMEOUT 30000  // 30 seconds timeout for initial connection

DHT dht(DHT22_PIN, DHTTYPE);
WiFiSSLClient wifiClient;
HttpClient httpClient(wifiClient, SERVER_URL, SERVER_PORT);

// NTP Settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;     // Adjust your GMT offset here
const int   daylightOffset_sec = 0; // Adjust for daylight savings if needed
WiFiUDP ntpUDP;

unsigned long previousMillis = 0;
const long interval = 5000; // 5 second interval between sensor readings
bool wifiConnected = false;

//#########################################################################################

void setup() {
  Serial.begin(9600);
  dht.begin(); // Initialize sensor

  // Attempt WiFi connection
  connectWiFi();

  // Sync time using NTP
  setSyncProvider(getNtpTime);
  if (timeStatus() == timeSet) {
    logToSerial("Time synchronized");
  } else {
    logToSerial("Failed to synchronize time");
  }
}


void loop() {
  unsigned long currentMillis = millis();

  // Check if it's time to take a reading
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      logToSerial("Failed to read sensor");
      return;
    }

    logToSerial("Temperature: " + String(temperature));
    logToSerial("Humidity: " + String(humidity));

    // Build JSON packet
    StaticJsonDocument<256> jsonDoc;
    jsonDoc["deviceId"] = DEVICE_ID;
    jsonDoc["modelType"] = MODEL_TYPE;
    jsonDoc["firmwareVersion"] = FIRMWARE_VERSION;
    jsonDoc["temperature"] = temperature;
    jsonDoc["humidity"] = humidity;
    jsonDoc["timestamp"] = now();  // Time in seconds since epoch

    String packetData;
    serializeJson(jsonDoc, packetData);

    sendSensorData(packetData);
  }

  // Monitor WiFi connection and try to reconnect if lost
  if (WiFi.status() != WL_CONNECTED) {
    logToSerial("WiFi disconnected. Attempting to reconnect...");
    connectWiFi();
  }
}

//#########################################################################################

void connectWiFi() {
  logToSerial("Attempting WiFi connection to " + String(WIFI_SSID));
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT) {
    Serial.print(".");
    delay(1000);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    logToSerial("WiFi connected successfully");
    logToSerial("Got IP: " + WiFi.localIP().toString());
    logToSerial("Signal strength: " + String(WiFi.RSSI()));
    wifiConnected = true;
  } else {
    logToSerial("WiFi connection failed, will retry later");
    wifiConnected = false;
  }
}


// Retrieve the current time from the NTP server
time_t getNtpTime() {
  const int NTP_PACKET_SIZE = 48;
  byte packetBuffer[NTP_PACKET_SIZE];
  memset(packetBuffer, 0, NTP_PACKET_SIZE);

  // Initialize NTP request packet
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // rest of the packet is already 0

  ntpUDP.begin(2390); // Use a random local port
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


void sendSensorData(String packetData) {
  if (!wifiConnected) {
    logToSerial("Not connected to WiFi, data not sent");
    return;
  }
  logToSerial("Sending data to server...");
  logToSerial(String(packetData)); // Prints json string to serial for debugging purposes

//############ OLD ########################################################################

  httpClient.beginRequest();
  httpClient.post(API_ROUTE);
  httpClient.sendHeader("Content-Type", "application/json");
  httpClient.sendHeader("Content-Length", packetData.length());
  httpClient.beginBody();
  httpClient.print(packetData);
  httpClient.endRequest();

//############ NEW ########################################################################

  /*if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
    logToSerial("Error: Failed to connect to server!");
    return;
  }
 
  // Send manual HTTP request
  wifiClient.println("POST " + String(API_ROUTE) + " HTTP/1.1");
  wifiClient.println("Host: " + String(SERVER_URL));
  wifiClient.println("Content-Type: application/json");
  wifiClient.println("Content-Length: " + String(packetData.length()));
  wifiClient.println("Connection: close");
  wifiClient.println();
  wifiClient.println(packetData);*/

//#########################################################################################

  // API response
  int statusCode = httpClient.responseStatusCode();
  String response = httpClient.responseBody();
  logToSerial("Status Code: " + String(statusCode));
  logToSerial("Response: " + String(response));
}


void logToSerial(String logMessage) {
  int messageLength = logMessage.length();
  String messageBorder = "";

  // Generate a string of '=' characters matching the length of logMessage
  for (int i = 0; i < messageLength; i++) {
    messageBorder += "=";
  }

  Serial.println("¤" + messageBorder + "¤");
  Serial.println("|" + logMessage + "|");
  Serial.println("¤" + messageBorder + "¤");
}
