#include "WiFiS3.h"
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

// Timing settings
#define API_TIMEOUT  10000 // 10 seconds
#define WIFI_TIMEOUT 20000 // 20 seconds
#define LOOP_INTERVAL 5000 //  5 seconds

DHT dht(DHT22_PIN, DHTTYPE);
WiFiSSLClient wifiClient;

// NTP Settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;
WiFiUDP ntpUDP;

unsigned long previousMillis = 0;

//#########################################################################################

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
    logToSerial("Time synchronized");
  } else {
    logToSerial("Failed to synchronize time");
  }
}


void loop() {
  unsigned long currentMillis = millis();

  // Check if it's time to take a reading
  if (currentMillis - previousMillis > LOOP_INTERVAL) {
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
  } else {
    logToSerial("WiFi connection failed, will retry later");
  }
}

//#########################################################################################

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

//#########################################################################################

// Sends the sensor data to the server using a http post request
void sendSensorData(String packetData) {
  logToSerial("Sending data to server...");
  logToSerial(packetData); // Log the JSON payload
  
  if (!wifiClient.connect(SERVER_URL, SERVER_PORT)) {
    logToSerial("Error: Failed to connect to server");
    return;
  }
 
  // HTTP POST request
  // TODO: Refactor this bit into it's own function
  String httpRequest =
    "POST "            + String(API_ROUTE) + " HTTP/1.1" + "\r\n" +
    "Host: "           + String(SERVER_URL)              + "\r\n" +
    "Content-Type: "   + "application/json"              + "\r\n" +
    "Content-Length: " + String(packetData.length())     + "\r\n" +
    "Connection: "     + "close"                     + "\r\n\r\n" +
    packetData;
    
  wifiClient.print(httpRequest);
  
  // Wait for a response with a timeout of 5 seconds
  unsigned long timeout = millis();
  while (wifiClient.available() == 0) {
    if (millis() - timeout > API_TIMEOUT) {
      logToSerial("Error: Server response timed out.");
      wifiClient.stop();
      return;
    }
  }
  
  // Read API response from the server
  String response = "";
  while (wifiClient.available()) {
    response += (char)wifiClient.read();
  }
  logToSerial("Response: " + response);
  
  // Close the connection to free resources
  wifiClient.stop();
}

//#########################################################################################

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
