#include "WebServer.h"

WebServer::WebServer(FancyLog& log, NetworkManager& network)
    : fancyLog(log), networkManager(network), server(80) {}

void WebServer::begin() {
    server.begin();
    fancyLog.toSerial("WEB server initialized");
}

void WebServer::handleClient() {
    WiFiClient client = server.available();

    if (client) {
        String currentLine = "";
        String request = "";
        unsigned long timeout = millis();

        while (client.connected() && millis() - timeout < 3000) {
            if (client.available()) {
                char c = client.read();
                request += c;

                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        // HTTP header is complete, process the request
                        if (request.startsWith("GET / ") || request.startsWith("GET /index.html ")) {
                            handleRoot(client);
                        }
                        else if (request.startsWith("GET /settings ")) {
                            handleSettings(client);
                        }
                        else if (request.startsWith("POST /save-settings ")) {
                            handleSaveSettings(client, request);
                        }
                        else {
                            handleNotFound(client);
                        }
                        break;
                    } else {
                        currentLine = "";
                    }
                } else if (c != '\r') {
                    currentLine += c;
                }
            }
        }

        delay(1);
        client.stop();
        fancyLog.toSerial("Client disconnected", DEBUG);
    }
}

void WebServer::handleRoot(WiFiClient& client) {
    fancyLog.toSerial("Handling root request", DEBUG);
    sendHeader(client, "H2Climate Device Control Panel");

    // Device information
    client.println("<div class='card'>");
    client.println("<h2>Device Information</h2>");
    client.println("<p><strong>Device ID:</strong> " + String(DeviceIdentifier::getDeviceId()) + "</p>");
    client.println("<p><strong>Model:</strong> " + String(MODEL_TYPE) + "</p>");
    client.println("<p><strong>Firmware:</strong> " + String(FIRMWARE_VERSION) + "</p>");
    client.println("<p><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</p>");
    client.println("</div>");

    // Quick actions
    client.println("<div class='card'>");
    client.println("<h2>Quick Actions</h2>");
    client.println("<div class='button-row'>");
    client.println("<a href='/settings' class='button'>Network Settings</a>");
    client.println("<a href='/data' class='button'>View Data</a>");
    client.println("</div>");
    client.println("</div>");

    // Status
    client.println("<div class='card'>");
    client.println("<h2>Status</h2>");
    client.println("<p><strong>WiFi:</strong> " + String(WiFi.SSID()) + " (" + (networkManager.isConnected() ? "Connected" : "Disconnected") + ")</p>");
    client.println("<p><strong>RSSI:</strong> " + String(WiFi.RSSI()) + " dBm</p>");
    client.println("<p><strong>Uptime:</strong> " + String(millis() / 1000) + " seconds</p>");
    client.println("</div>");

    sendFooter(client);
}

void WebServer::handleSettings(WiFiClient& client) {
    fancyLog.toSerial("Handling settings request", DEBUG);
    sendHeader(client, "Network Settings");

    client.println("<div class='card'>");
    client.println("<h2>WiFi Configuration</h2>");
    client.println("<form method='post' action='/save-settings'>");
    client.println("<div class='form-group'>");
    client.println("<label for='ssid'>WiFi SSID:</label>");
    client.println("<input type='text' id='ssid' name='ssid' value='" + String(WIFI_SSID) + "'>");
    client.println("</div>");
    client.println("<div class='form-group'>");
    client.println("<label for='password'>WiFi Password:</label>");
    client.println("<input type='password' id='password' name='password' placeholder='Enter new password'>");
    client.println("</div>");
    client.println("<div class='form-group'>");
    client.println("<button type='submit' class='button'>Save Settings</button>");
    client.println("<a href='/' class='button secondary'>Cancel</a>");
    client.println("</div>");
    client.println("</form>");
    client.println("</div>");

    sendFooter(client);
}

void WebServer::handleSaveSettings(WiFiClient& client, String request) {
    fancyLog.toSerial("Handling save settings request", DEBUG);

    // Parse form data
    int bodyStart = request.indexOf("\r\n\r\n") + 4;
    String body = request.substring(bodyStart);

    // Parse SSID
    int ssidStart = body.indexOf("ssid=") + 5;
    int ssidEnd = body.indexOf("&", ssidStart);
    String ssid = urlDecode(body.substring(ssidStart, ssidEnd));

    // Parse password
    int passwordStart = body.indexOf("password=", ssidEnd) + 9;
    String password = urlDecode(body.substring(passwordStart));

    // Save credentials if both parameters are present
    if (ssid.length() > 0 && password.length() > 0) {
        saveNetworkCredentials(ssid.c_str(), password.c_str());

        sendHeader(client, "Settings Saved");
        client.println("<div class='card'>");
        client.println("<h2>Settings Saved Successfully</h2>");
        client.println("<p>The device will restart to apply the new network settings.</p>");
        client.println("<p><strong>New SSID:</strong> " + ssid + "</p>");
        client.println("<div class='button-row'>");
        client.println("<a href='/' class='button'>Back to Home</a>");
        client.println("</div>");
        client.println("</div>");
        sendFooter(client);

        // Schedule a restart after the response is sent
        delay(1000);
        NVIC_SystemReset(); // Resets arduino board
    } else {
        sendHeader(client, "Error");
        client.println("<div class='card'>");
        client.println("<h2>Error Saving Settings</h2>");
        client.println("<p>Both SSID and password must be provided.</p>");
        client.println("<div class='button-row'>");
        client.println("<a href='/settings' class='button'>Try Again</a>");
        client.println("</div>");
        client.println("</div>");
        sendFooter(client);
    }
}

void WebServer::handleNotFound(WiFiClient& client) {
    sendHeader(client, "Page Not Found");
    client.println("<div class='card'>");
    client.println("<h2>404 - Page Not Found</h2>");
    client.println("<p>The page you requested could not be found.</p>");
    client.println("<div class='button-row'>");
    client.println("<a href='/' class='button'>Back to Home</a>");
    client.println("</div>");
    client.println("</div>");
    sendFooter(client);
}

void WebServer::sendHeader(WiFiClient& client, const char* title) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();

    client.println("<!DOCTYPE html>");
    client.println("<html lang='en'>");
    client.println("<head>");
    client.println("<meta charset='UTF-8'>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    client.println("<title>H2Climate - " + String(title) + "</title>");
    client.println("<style>");
    client.println("body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f5f5f5; color: #333; }");
    client.println("header { background-color: #2c3e50; color: white; padding: 1rem; text-align: center; }");
    client.println(".container { max-width: 800px; margin: 0 auto; padding: 1rem; }");
    client.println(".card { background-color: white; border-radius: 4px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); margin-bottom: 1rem; padding: 1rem; }");
    client.println("h1 { margin: 0; }");
    client.println("h2 { margin-top: 0; color: #2c3e50; }");
    client.println(".button-row { display: flex; flex-wrap: wrap; gap: 0.5rem; margin-top: 1rem; }");
    client.println(".button { background-color: #3498db; color: white; padding: 0.5rem 1rem; text-decoration: none; border-radius: 4px; display: inline-block; border: none; cursor: pointer; }");
    client.println(".button.secondary { background-color: #95a5a6; }");
    client.println(".button:hover { background-color: #2980b9; }");
    client.println(".button.secondary:hover { background-color: #7f8c8d; }");
    client.println(".form-group { margin-bottom: 1rem; }");
    client.println("label { display: block; margin-bottom: 0.5rem; font-weight: bold; }");
    client.println("input { width: 100%; padding: 0.5rem; border: 1px solid #ddd; border-radius: 4px; }");
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");
    client.println("<header>");
    client.println("<h1>H2Climate Device</h1>");
    client.println("</header>");
    client.println("<div class='container'>");
    client.println("<h2>" + String(title) + "</h2>");
}

void WebServer::sendFooter(WiFiClient& client) {
    client.println("</div>");
    client.println("<footer style='text-align: center; padding: 1rem; color: #7f8c8d; font-size: 0.8rem;'>");
    client.println("H2Climate &copy; " + String(year()) + " | Firmware " + String(FIRMWARE_VERSION));
    client.println("</footer>");
    client.println("</body>");
    client.println("</html>");
}

String WebServer::getContentType(String filename) {
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".json")) return "application/json";
    return "text/plain";
}

String WebServer::urlDecode(String text) {
    String decoded = "";
    char temp[] = "0x00";

    for (unsigned int i = 0; i < text.length(); i++) {
        if (text[i] == '+') {
            decoded += ' ';
        } else if (text[i] == '%') {
            temp[2] = text[i+1];
            temp[3] = text[i+2];
            decoded += (char)strtol(temp, NULL, 16);
            i += 2;
        } else {
            decoded += text[i];
        }
    }

    return decoded;
}

void WebServer::saveNetworkCredentials(const char* ssid, const char* password) {
    // This is a placeholder - you'll need to implement a proper method
    // to save credentials to EEPROM or flash memory
    fancyLog.toSerial("Saving new network credentials: " + String(ssid), INFO);

    // You'll need to implement this based on your config storage system
    // For now, we'll just print the values
    fancyLog.toSerial("New SSID: " + String(ssid), INFO);
    fancyLog.toSerial("New password saved (not shown for security)", INFO);

    // After saving credentials, you might want to restart the device
    // or reconnect to WiFi with the new credentials
}

