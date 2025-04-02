#include "WiFiS3.h"
#include "ArduinoHttpClient.h"
#include "DHT.h"
#include "secrets.h" // WiFi credentials

// Test device ID: 6fe26f8eaf7e

// Sensor details
#define DHT22_PIN 12
#define DHTTYPE DHT22

// Server details
const char* serverUrl = "blabla.com";
int serverPort = 80;  // Refactor to https later

DHT dht(DHT22_PIN, DHTTYPE);

WiFiClient wifiClient;
HttpClient httpClient(wifiClient, serverUrl, serverPort);


void setup() {

    Serial.begin(9600);
    dht.begin();
    
    // Connect to WiFi
    Serial.print("Attempting WiFi connection");
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000); // 1 second delay
        Serial.print(".");
    }

    Serial.println("\nWiFi connected successfully");
}


void loop() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Failed to read sensor");
        delay(2000); // 2 second delay
        return;
    }

    Serial.print("Temperature: "); Serial.println(temperature);
    Serial.print("Humidity: "); Serial.println(humidity);

    // Send data to API
    // TODO: add error handling
    sendSensorData(temperature, humidity);
    
    delay(5000);  // 5 second delay
}


void sendSensorData(float temperature, float humidity) {
    // JSON packet
    String packetData = "{\"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + "}";

    Serial.println("Sending data to server...");
    httpClient.beginRequest();
    httpClient.post("");  // TODO: add API route
    httpClient.sendHeader("Content-Type", "application/json");
    httpClient.sendHeader("Content-Length", packetData.length());
    httpClient.beginBody();
    httpClient.print(packetData);
    httpClient.endRequest();

    // API response
    int statusCode = httpClient.responseStatusCode();
    String response = httpClient.responseBody();

    Serial.print("Status Code: "); Serial.println(statusCode);
    Serial.print("Response: "); Serial.println(response);
}