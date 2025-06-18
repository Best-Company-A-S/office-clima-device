/*
 * H2Climate Device Firmware V1.0.0
 * For Arduino UNO R4 WiFi
 * 
 * This firmware provides temperature and humidity monitoring with
 * automatic firmware updates, LED matrix status display, and battery monitoring.
 */

#include "src/network/NetworkManager.h"
#include "src/sensors/SensorManager.h"
#include "src/utils/BatteryMonitor.h"
#include "src/network/WebServer.h"
#include "src/network/BluetoothManager.h"
#include "src/utils/EEPROMManager.h" // Include the new EEPROMManager

// Global objects
FancyLog fancyLog;
OTAManager otaManager;
SensorManager sensors(fancyLog);
DisplayManager display(fancyLog);
BatteryMonitor battery(fancyLog);
EEPROMManager eepromManager(fancyLog); // Create EEPROMManager instance
NetworkManager network(fancyLog, otaManager, display);
DeviceIdentifier deviceID;
WebServer webServer(fancyLog, network);
BluetoothManager bluetooth(fancyLog);

// Timing variables
unsigned long previousMillis = 0;
unsigned long previousUpdateCheckMillis = 0;
unsigned long previousBatteryLogMillis = 0;
unsigned long previousBluetoothCheckMillis = 0;

// Data collection variables
int dataCount = 0;
struct SensorData {
  	float temperature;
  	float humidity;
  	float batteryVoltage;
  	int batteryPercentage;
  	int batteryTimeRemaining;
  	unsigned long timestamp;
};
SensorData dataBuffer[DATA_BUFFER_SIZE]; // Using DATA_BUFFER_SIZE defined in Config.h

// Call it *only once*! Comment it out after the first run to clear EEPROM
// void clearAllEEPROM() {
//     eepromManager.clearEEPROM();
// }

//¤============¤
//| Setup loop |
//¤============¤==========================================================================¤
void setup() {
  	fancyLog.begin(9600); // Initialize serial connection and logger

  	// Initialize EEPROM manager
  	eepromManager.begin();

  	// Connect EEPROMManager to components that need it
  	DeviceIdentifier::setEEPROMManager(&eepromManager);
  	network.setEEPROMManager(&eepromManager);
  	webServer.setEEPROMManager(&eepromManager);

	// Calling getDeviceId also initialize the DeviceIdentifier
    fancyLog.toSerial("Starting H2Climate Device | ID: " + String(deviceID.getDeviceId()), INFO);

  	display.begin(); // Initialize LED matrix
    battery.begin(); // Initialize battery monitoring
  	sensors.begin(); // Initialize sensors

    // Initialize Bluetooth for WiFi configuration
    bluetooth.begin();

    // Enter configuration mode if no stored Wi-Fi credentials or WiFi connection fails
    char ssid[33] = {0};
    char password[64] = {0};

    bool hasCredentials = network.loadWiFiCredentials(ssid, password);
    if (!hasCredentials) {
        enterConfigurationMode();
    } else {
        // Try to connect with stored credentials
        if (!network.connectWiFi(ssid, password)) {
            // If connection fails, enter configuration mode
            enterConfigurationMode();
        }
    }

  	webServer.begin(); // Initialize web server
  	network.registerDevice(); // Register device with server
  	network.checkForUpdates(); // Initial update check
    sensors.testSensors(); // attempt some initial test readings of the sensors to ensure they are working

    if (network.isConnected()) {
        fancyLog.toSerial("Setup complete: Web interface available at http://" + WiFi.localIP().toString() + ":80", INFO);
    } else {
        fancyLog.toSerial("Setup complete: Device in configuration mode", INFO);
    }
}

//¤==============¤
//| Runtime loop |
//¤==============¤========================================================================¤
void loop() {
  	// Handle OTA updates
  	network.pollOTA();

  	// Handle web server client requests
  	webServer.handleClient();

  	unsigned long currentMillis = millis();
  	unsigned long timeUntilNextReading = 0;

  	// Check WiFi connection
  	if (!network.isConnected()) {
    	display.showSadFace();

    	// Check for Bluetooth Wi-Fi configuration
    	if (currentMillis - previousBluetoothCheckMillis >= 1000) {
    	    previousBluetoothCheckMillis = currentMillis;
    	    bluetooth.poll();

    	    // If credentials received via Bluetooth, try to connect
    	    if (bluetooth.hasWifiCredentials()) {
    	        char newSsid[33] = {0};
    	        char newPassword[64] = {0};
    	        bluetooth.getWifiCredentials(newSsid, newPassword);

    	        fancyLog.toSerial("Received WiFi credentials via Bluetooth. Attempting to connect...", INFO);
    	        if (network.connectWiFi(newSsid, newPassword)) {
    	            // Successfully connected with new credentials
    	            bluetooth.resetCredentialFlag();
    	            fancyLog.toSerial("Successfully connected with new WiFi credentials", INFO);
    	            fancyLog.toSerial("Web interface available at http://" + WiFi.localIP().toString() + ":80", INFO);
    	        }
    	    }
    	}
  	}

  	// Calculate time until next reading
  	/*if (currentMillis - previousMillis < LOOP_INTERVAL) {
    	timeUntilNextReading = LOOP_INTERVAL - (currentMillis - previousMillis);

    	// Every 10 seconds, show time remaining until next data transmission
    	if (timeUntilNextReading % 10000 < 100) {
      		fancyLog.toSerial("Next transmission in " + String(timeUntilNextReading / 1000) + "s");
    	}
  	}*/

  	// Regular sensor readings and data transmission
  	if (currentMillis - previousMillis >= LOOP_INTERVAL) {
    	previousMillis = currentMillis;

    	fancyLog.toSerial("Taking sensor readings", INFO);

    	// Read sensor data
    	float temperature = sensors.readTemperature();
    	float humidity = sensors.readHumidity();
    	float batteryVoltage = battery.readVoltage();
    	int batteryPercentage = battery.readPercentage(batteryVoltage);
    	int batteryTimeRemaining = battery.estimateTimeRemaining(batteryPercentage);

    	if (isnan(temperature) || isnan(humidity)) {
      		fancyLog.toSerial("Failed to read sensor", ERROR);
      		display.showSadFace();
      		return;
    	}

    	// Store in buffer
    	dataBuffer[dataCount] = {
      		temperature,
      		humidity,
      		batteryVoltage,
      		batteryPercentage,
      		batteryTimeRemaining,
      		now()  // Use Unix timestamp from TimeLib instead of millis()
    	};
    	dataCount++;

    	// If buffer is full, send data
    	if (dataCount >= DATA_BUFFER_SIZE) {
      		sendBufferedData();
      		dataCount = 0;
    	}

    	// Show happy face unless battery is low
    	if (battery.isLowBattery()) {
      		display.showNeutralFace(); // Use neutral face for low battery
    	} else {
      		display.showHappyFace();
    	}
  	}

  	// Log battery status periodically
  	/*if (currentMillis - previousBatteryLogMillis >= BATTERY_LOG_INTERVAL) {
    	previousBatteryLogMillis = currentMillis;
    	battery.logStatus();
  	}*/

  	// Check for updates periodically
  	if (currentMillis - previousUpdateCheckMillis >= CHECK_INTERVAL) {
    	previousUpdateCheckMillis = currentMillis;
    	network.checkForUpdates();
  	}
}

void sendBufferedData() {
  	fancyLog.toSerial("Sending data", INFO);

    // Get the values we're sending
    float temp = dataBuffer[0].temperature;
    float hum = dataBuffer[0].humidity;
    float batteryVoltage = dataBuffer[0].batteryVoltage;
    int batteryPercentage = dataBuffer[0].batteryPercentage;
    int batteryTimeRemaining = dataBuffer[0].batteryTimeRemaining;
    unsigned long timestamp = dataBuffer[0].timestamp;

    // Create the JSON document
    StaticJsonDocument<384> jsonDoc;
    jsonDoc["deviceId"] = DeviceIdentifier::getDeviceId();
    jsonDoc["temperature"] = temp;
    jsonDoc["humidity"] = hum;
    jsonDoc["batteryVoltage"] = batteryVoltage;
    jsonDoc["batteryPercentage"] = batteryPercentage;
    jsonDoc["batteryTimeRemaining"] = batteryTimeRemaining;
    jsonDoc["timestamp"] = timestamp;

  	String sensorData;
  	serializeJson(jsonDoc, sensorData);

  	// Log the exact JSON format
  	fancyLog.toSerial("JSON Format: " + sensorData, INFO);

  	if (network.sendHttpPostRequest(sensorData, API_DATA_ROUTE)) {
    	fancyLog.toSerial("Data sent successfully", INFO);
  	} else {
    	fancyLog.toSerial("Failed to send data", ERROR);
  	}
}

// Helper function to enter configuration mode
void enterConfigurationMode() {
    fancyLog.toSerial("Entering WiFi configuration mode via Bluetooth", INFO);
    display.showNeutralFace();

    // Wait for credentials via Bluetooth
    unsigned long startWaitTime = millis();
    boolean connected = false;

    while (!connected && (millis() - startWaitTime < 300000)) { // Wait for 5 minutes max
        // Poll Bluetooth for credentials
        bluetooth.poll();

        if (bluetooth.hasWifiCredentials()) {
            char newSsid[33] = {0};
            char newPassword[64] = {0};
            bluetooth.getWifiCredentials(newSsid, newPassword);

            fancyLog.toSerial("Received WiFi credentials via Bluetooth. Attempting to connect...", INFO);

            if (network.connectWiFi(newSsid, newPassword)) {
                // Successfully connected with new credentials
                connected = true;
                bluetooth.resetCredentialFlag();
                network.begin(); // Initialize network components now that we're connected
                break;
            } else {
                // Failed to connect, wait for new credentials
                bluetooth.resetCredentialFlag();
                fancyLog.toSerial("Failed to connect with provided credentials. Please try again.", WARNING);
            }
        }

        // Show a visual indicator that we're in configuration mode
        if ((millis() / 500) % 2 == 0) {
            display.showNeutralFace();
        } else {
            display.showSadFace();
        }

        delay(100);
    }

    if (!connected) {
        // If we timed out without connecting, fall back to default credentials
        fancyLog.toSerial("Configuration timeout. Falling back to default WiFi credentials.", WARNING);
        network.begin();
    }
}
