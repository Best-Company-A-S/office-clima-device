/*
 * H2Climate Device Firmware v0.6.1
 * For Arduino UNO R4 WiFi
 * 
 * This firmware provides temperature and humidity monitoring with
 * automatic firmware updates and LED matrix status display.
 */

#include "src/config/Config.h"
#include "src/display/DisplayManager.h"
#include "src/network/NetworkManager.h"
#include "src/sensors/SensorManager.h"
#include "src/utils/Logger.h"
#include "src/config/secrets.h"
#include "src/config/params.h"

//¤=======================================================================================¤
//| TODO: Battery logging                                                                 |
//| TODO: Add sound sensor                                                               |
//| TODO: Changeable settings                                                            |
//| TODO: Use MAC address to make a unique DEVICE_ID                                     |
//| TODO: Add warning triggers at certain temperatures and humidities                    |
//| TODO: Store more sensor data before sending a packet to reduce packet spam           |
//¤=======================================================================================¤

// Global objects
Logger logger;
DisplayManager display;
NetworkManager network(display, logger);
SensorManager sensors(logger);

// Timing variables
unsigned long previousMillis = 0;
unsigned long previousUpdateCheckMillis = 0;

// Data collection variables
const int DATA_BUFFER_SIZE = 1;  // Number of readings to store before sending
int dataCount = 0;
struct SensorData {
  float temperature;
  float humidity;
  unsigned long timestamp;
};
SensorData dataBuffer[DATA_BUFFER_SIZE];

void setup() {
  // Initialize components
  logger.begin(9600);
  logger.logWithBorder("Starting H2Climate Device");
  logger.logWithBorder("Device ID: " + String(DEVICE_ID));

  // Initialize LED matrix
  display.begin();
  display.showNeutralFace();  // Show neutral face during setup

  // Initialize sensors - using exact code pattern from original
  logger.logWithBorder("Initializing sensors");
  sensors.begin();

  // Connect to network after sensors are initialized
  network.begin();

  // Register device with server
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["deviceId"] = DEVICE_ID;
  jsonDoc["modelType"] = MODEL_TYPE;
  jsonDoc["firmwareVersion"] = FIRMWARE_VERSION;

  String registerData;
  serializeJson(jsonDoc, registerData);
  network.sendHttpPostRequest(registerData, API_REGISTER_ROUTE);

  // Initial update check
  network.checkForUpdates();
  logger.logWithBorder("Setup complete");
}

void loop() {
  // Handle OTA updates
  network.pollOTA();

  unsigned long currentMillis = millis();
  unsigned long timeUntilNextReading = 0;

  // Check WiFi connection
  if (!network.isConnected()) {
    display.showSadFace();
    logger.logWithBorder("WiFi disconnected. Reconnecting...");
    network.connectWiFi();
  }

  // Calculate time until next reading
  if (currentMillis - previousMillis < LOOP_INTERVAL) {
    timeUntilNextReading = LOOP_INTERVAL - (currentMillis - previousMillis);

    // Every 10 seconds, show time remaining until next data transmission
    if (timeUntilNextReading % 10000 < 100) {
      logger.log("Next transmission in " + String(timeUntilNextReading / 1000) + "s");
    }
  }

  // Regular sensor readings and data transmission
  if (currentMillis - previousMillis >= LOOP_INTERVAL) {
    previousMillis = currentMillis;

    logger.logWithBorder("Taking sensor readings");

    // Read sensor data directly like original code
    float temperature = sensors.readTemperature();
    float humidity = sensors.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      logger.logWithBorder("Failed to read sensor");
      display.showSadFace();
      return;
    }

    // Display with consistent decimal places
    logger.logWithBorder("Temp: " + String(temperature, 1) + "°C, Humidity: " + String(humidity, 1) + "%");

    // Store in buffer
    dataBuffer[dataCount] = {
      temperature,
      humidity,
      now()  // Use Unix timestamp from TimeLib instead of millis()
    };
    dataCount++;

    // If buffer is full, send data
    if (dataCount >= DATA_BUFFER_SIZE) {
      sendBufferedData();
      dataCount = 0;
    }

    display.showHappyFace();
  }

  // Check for updates periodically
  if (currentMillis - previousUpdateCheckMillis >= CHECK_INTERVAL) {
    previousUpdateCheckMillis = currentMillis;
    network.checkForUpdates();
  }
}

void sendBufferedData() {
  // Get the values we're sending
  float temp = dataBuffer[0].temperature;
  float hum = dataBuffer[0].humidity;
  unsigned long timestamp = dataBuffer[0].timestamp;

  // Create the JSON document
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["deviceId"] = DEVICE_ID;
  jsonDoc["temperature"] = temp;
  jsonDoc["humidity"] = hum;
  jsonDoc["timestamp"] = timestamp;

  String sensorData;
  serializeJson(jsonDoc, sensorData);

  // Log the exact JSON format
  logger.log("JSON Format: " + sensorData);

  if (network.sendHttpPostRequest(sensorData, API_DATA_ROUTE)) {
    logger.logWithBorder("Data sent successfully");
  } else {
    logger.logWithBorder("Failed to send data");
  }
}