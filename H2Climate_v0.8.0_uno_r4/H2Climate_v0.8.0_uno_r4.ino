/*
 * H2Climate Device Firmware v0.8.0
 * For Arduino UNO R4 WiFi
 * 
 * This firmware provides temperature and humidity monitoring with
 * automatic firmware updates, LED matrix status display, and battery monitoring.
 */

#include "src/network/NetworkManager.h"
#include "src/sensors/SensorManager.h"
#include "src/utils/BatteryMonitor.h"
#include "src/network/WebServer.h" // Add WebServer include

// Global objects
FancyLog fancyLog;
OTAManager otaManager;
DisplayManager display;
NetworkManager network(fancyLog, otaManager, display);
SensorManager sensors(fancyLog);
BatteryMonitor battery(fancyLog);
DeviceIdentifier deviceID;
WebServer webServer(fancyLog, network); // Add WebServer object

// Timing variables
unsigned long previousMillis = 0;
unsigned long previousUpdateCheckMillis = 0;
unsigned long previousBatteryLogMillis = 0;

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


void clearEEPROM() {
    for (int i = 0; i < EEPROM.length(); i++) {
        EEPROM.write(i, 0xFF); // Writes 0xFF (default empty value) to the entire EEPROM
    }
    Serial.println("EEPROM cleared");
}


//¤============¤
//| Setup loop |
//¤============¤==========================================================================¤
void setup() {
  	// Initialize serial connection and logger
  	fancyLog.begin(9600);
  	fancyLog.toSerial("Serial connection initialized", INFO);


	//clearEEPROM(); // Call it *only once*! Comment it out after the first run


	// Calling getDeviceId also initialize the DeviceIdentifier
    fancyLog.toSerial("Starting H2Climate Device | ID: " + String(deviceID.getDeviceId()), INFO);

  	// Initialize LED matrix
  	display.begin();
  	display.showNeutralFace();  // Show neutral face during setup

  	// Initialize sensors
  	fancyLog.toSerial("Initializing sensors", INFO);
  	sensors.begin();
  
  	// Initialize battery monitoring
  	fancyLog.toSerial("Initializing battery monitoring", INFO);
  	battery.begin();

  	// Connect to network after sensors are initialized
  	network.begin();

  	// Register device with server
  	StaticJsonDocument<256> jsonDoc;
  	jsonDoc["deviceId"] = DeviceIdentifier::getDeviceId();
  	jsonDoc["modelType"] = MODEL_TYPE;
  	jsonDoc["firmwareVersion"] = FIRMWARE_VERSION;

  	String registerData;
  	serializeJson(jsonDoc, registerData);
  	network.sendHttpPostRequest(registerData, API_REGISTER_ROUTE);

  	// Initial update check
  	network.checkForUpdates();

  	// Initialize web server
  	webServer.begin();
  	fancyLog.toSerial("Web interface available at http://" + WiFi.localIP().toString(), INFO);

  	fancyLog.toSerial("Setup complete", INFO);
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
    	fancyLog.toSerial("WiFi disconnected. Reconnecting...", WARNING);
    	network.connectWiFi();
  	}

  	// Calculate time until next reading
  	if (currentMillis - previousMillis < LOOP_INTERVAL) {
    	timeUntilNextReading = LOOP_INTERVAL - (currentMillis - previousMillis);

    	// Every 10 seconds, show time remaining until next data transmission
    	if (timeUntilNextReading % 10000 < 100) {
      		fancyLog.toSerial("Next transmission in " + String(timeUntilNextReading / 1000) + "s");
    	}
  	}

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
  	if (currentMillis - previousBatteryLogMillis >= BATTERY_LOG_INTERVAL) {
    	previousBatteryLogMillis = currentMillis;
    	battery.logStatus();
  	}

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