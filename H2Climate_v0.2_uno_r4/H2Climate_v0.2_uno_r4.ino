#include "WiFiS3.h"
#include "ArduinoHttpClient.h"
#include "DHT.h"
#include "secrets.h" // WiFi credentials

// Sensor details
#define DHT22_PIN 12
#define DHTTYPE DHT22

// Server details
#define SERVER_URL "bestcompany.laravel.cloud"
#define SERVER_PORT 443
#define API_ROUTE "/api/devices/data"
//const char* serverUrl = "bestcompany.laravel.cloud";
//int serverPort = 443;  // Refactor to https later

DHT dht(DHT22_PIN, DHTTYPE);

WiFiSSLClient wifiClient;
HttpClient httpClient(wifiClient, SERVER_URL, SERVER_PORT);
//HttpClient httpClient(wifiClient, serverUrl, serverPort);


void setup() {

    Serial.begin(9600);
    dht.begin();
    
    // Connect to WiFi
    Serial.print("Attempting WiFi connection to "); Serial.println(String(WIFI_SSID) + " network");
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000); // 1 second delay
    }

    Serial.println("WiFi connected successfully");

    // shows the server's ip and it's wifi signal strength
    Serial.print("\nGot IP: "); Serial.println(WiFi.localIP());
    Serial.print("Signal strength: "); Serial.println(WiFi.RSSI());
    Serial.print("\n");
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
    Serial.print("Humidity: "); Serial.println(String(humidity) + "\n");

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

    httpClient.post(API_ROUTE);
    //httpClient.post("/api/devices/data");

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