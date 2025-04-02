#include <WiFiS3.h>

#define WIFI_SSID "DEV"
#define WIFI_PASS "Kaffe10ko"
#define SERVER_PORT 80
#define LED_PIN 2

WiFiServer server(SERVER_PORT);
bool LedStatus = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  int timeout = 30;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(1000);
    Serial.print(".");
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());
    server.begin();
    Serial.println("HTTP server started!");
  } else {
    Serial.println("\nFailed to connect to WiFi.");
  }
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected.");
    String request = "";
    while (client.available()) {
      char c = client.read();
      request += c;
      if (c == '\n') break;  // End of request line
    }

    Serial.println("Request received: " + request);

    // Handle LED Control
    if (request.indexOf("GET /led-on") != -1) {
      LedStatus = HIGH;
    } else if (request.indexOf("GET /led-off") != -1) {
      LedStatus = LOW;
    }
    digitalWrite(LED_PIN, LedStatus);

    // Send HTTP Response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println(SendHTML(LedStatus));
    
    client.stop();
    Serial.println("Client disconnected.");
  }
}

String SendHTML(uint8_t ledstat) {
  String html = "<!DOCTYPE html><html><head><title>LED Control</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'></head><body>";
  html += "<h1>Arduino UNO R4 Web Server</h1><h3>WiFi LED Control</h3>";
  
  if (ledstat) {
    html += "<p>LED Status: ON</p><a href='/led-off'><button>Turn OFF</button></a>";
  } else {
    html += "<p>LED Status: OFF</p><a href='/led-on'><button>Turn ON</button></a>";
  }
  
  html += "</body></html>";
  return html;
}

