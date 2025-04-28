/*
 * H2Climate Device Firmware v0.7.2
 * For Arduino UNO R4 WiFi
 * 
 * This firmware provides temperature and humidity monitoring with
 * automatic firmware updates, LED matrix status display, and battery monitoring.
 */

#include "src/config/Config.h"
#include "src/display/DisplayManager.h"
#include "src/network/NetworkManager.h"
#include "src/sensors/SensorManager.h"
#include "src/sensors/BatteryMonitor.h"
#include "src/utils/FancyLog.h"
#include "src/utils/DeviceIdentifier.h"
#include "src/config/secrets.h"

//¤=======================================================================================¤
//| TODO: Add sound sensor (Sound sensor is garbango so maybe not)                        |
//| TODO: Changeable settings                                                             |
//| TODO: Add warning triggers at certain temperatures and humidities                     |
//| TODO: Store more sensor data before sending a packet to reduce packet spam            |
//¤=======================================================================================¤

// Global objects
FancyLog fancyLog;
DisplayManager display;
NetworkManager network(display, fancyLog);
SensorManager sensors(fancyLog);
BatteryMonitor battery(fancyLog);

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
SensorData dataBuffer[DATA_BUFFER_SIZE]; // Using DATA_BUFFER_SIZE defined in params.h

void setup() {
  fancyLog.toSerial("Starting H2Climate Device", INFO);

  // Initialize components
  fancyLog.begin(9600);
  fancyLog.toSerial("Serial connection initialized", INFO);

  // Initialize device identifier
  DeviceIdentifier::initialize();
  DeviceIdentifier::printDeviceInfo();
  fancyLog.toSerial("Device ID: " + DeviceIdentifier::getDeviceId(), INFO);

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
  fancyLog.toSerial("Setup complete", INFO);
}

void loop() {
  // Handle OTA updates
  network.pollOTA();

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
    int batteryPercentage = battery.readPercentage();
    int batteryTimeRemaining = battery.estimateTimeRemaining();

    if (isnan(temperature) || isnan(humidity)) {
      fancyLog.toSerial("Failed to read sensor", ERROR);
      display.showSadFace();
      return;
    }

    // Display with consistent decimal places
    fancyLog.toSerial("Temp: " + String(temperature, 1) + "°C, Humidity: " + String(humidity, 1) + "%");

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
  // Get the values we're sending
  float temp = dataBuffer[0].temperature;
  float hum = dataBuffer[0].humidity;
  float batteryVoltage = dataBuffer[0].batteryVoltage;
  int batteryPercentage = dataBuffer[0].batteryPercentage;
  int batteryTimeRemaining = dataBuffer[0].batteryTimeRemaining;
  unsigned long timestamp = dataBuffer[0].timestamp;

  // Log battery data before sending to verify
  fancyLog.toSerial("Battery data to send - Voltage: " + String(batteryVoltage, 3) +
             "V, Percentage: " + String(batteryPercentage) + 
             "%, Time remaining: " + String(batteryTimeRemaining) + " minutes");

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
  fancyLog.toSerial("JSON Format: " + sensorData);

  if (network.sendHttpPostRequest(sensorData, API_DATA_ROUTE)) {
    fancyLog.toSerial("Data sent successfully", INFO);
  } else {
    fancyLog.toSerial("Failed to send data", ERROR);
  }
}