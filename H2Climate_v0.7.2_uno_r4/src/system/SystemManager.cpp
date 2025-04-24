#include "SystemManager.h"
#include "../display/DisplayManager.h"
#include "../network/NetworkManager.h"
#include "../sensors/SensorManager.h"
#include "../sensors/BatteryMonitor.h"
#include "../data/DataManager.h"
#include "../utils/DeviceIdentifier.h"

SystemManager::SystemManager(DisplayManager& display, NetworkManager& network, 
                             SensorManager& sensors, BatteryMonitor& battery,
                             DataManager& data, FancyLog& log)
  : displayManager(display), networkManager(network), 
    sensorManager(sensors), batteryMonitor(battery),
    dataManager(data), fancyLog(log),
    currentState(INITIALIZING),
    previousSensorReadMillis(0),
    previousUpdateCheckMillis(0),
    previousBatteryLogMillis(0) {
}

void SystemManager::begin() {
  fancyLog.toSerial("Starting H2Climate Device", INFO);
  
  // Initialize components
  fancyLog.begin(9600);
  fancyLog.toSerial("Serial connection initialized", INFO);
  
  // Initialize device identifier
  DeviceIdentifier::initialize();
  DeviceIdentifier::printDeviceInfo();
  fancyLog.toSerial("Device ID: " + DeviceIdentifier::getDeviceId(), INFO);
  
  // Initialize LED matrix
  displayManager.begin();
  displayManager.showNeutralFace();  // Show neutral face during setup
  
  // Initialize sensors
  fancyLog.toSerial("Initializing sensors", INFO);
  sensorManager.begin();
  
  // Initialize battery monitoring
  fancyLog.toSerial("Initializing battery monitoring", INFO);
  batteryMonitor.begin();
  
  // Connect to network after sensors are initialized
  networkManager.begin();
  
  // Register device with server
  registerDevice();
  
  // Initial update check
  networkManager.checkForUpdates();
  
  // Set state to running
  setState(RUNNING);
  
  fancyLog.toSerial("Setup complete", INFO);
}

void SystemManager::update() {
  unsigned long currentMillis = millis();
  
  // Handle OTA updates
  networkManager.pollOTA();
  
  // Check WiFi connection
  if (!networkManager.isConnected()) {
    setState(CONNECTING);
    displayManager.showSadFace();
    fancyLog.toSerial("WiFi disconnected. Reconnecting...", WARNING);
    networkManager.connectWiFi();
    if (networkManager.isConnected()) {
      setState(RUNNING);
    }
  }
  
  // Regular sensor readings and data transmission
  if (currentMillis - previousSensorReadMillis >= SENSOR_READ_INTERVAL) {
    previousSensorReadMillis = currentMillis;
    readSensors();
  }
  
  // Log battery status periodically
  if (currentMillis - previousBatteryLogMillis >= BATTERY_LOG_INTERVAL) {
    previousBatteryLogMillis = currentMillis;
    checkBattery();
  }
  
  // Check for updates periodically
  if (currentMillis - previousUpdateCheckMillis >= UPDATE_CHECK_INTERVAL) {
    previousUpdateCheckMillis = currentMillis;
    checkForUpdates();
  }
  
  // Update display based on current state
  updateDisplay();
}

SystemState SystemManager::getState() const {
  return currentState;
}

void SystemManager::setState(SystemState newState) {
  if (newState != currentState) {
    fancyLog.toSerial("System state changed to: " + String(newState), INFO);
    currentState = newState;
  }
}

void SystemManager::registerDevice() {
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["deviceId"] = DeviceIdentifier::getDeviceId();
  jsonDoc["modelType"] = MODEL_TYPE;
  jsonDoc["firmwareVersion"] = FIRMWARE_VERSION;
  
  String registerData;
  serializeJson(jsonDoc, registerData);
  networkManager.sendHttpPostRequest(registerData, API_REGISTER_ROUTE);
}

void SystemManager::readSensors() {
  fancyLog.toSerial("Taking sensor readings", INFO);
  
  // Read sensor data
  float temperature = sensorManager.readTemperature();
  float humidity = sensorManager.readHumidity();
  float batteryVoltage = batteryMonitor.readVoltage();
  int batteryPercentage = batteryMonitor.readPercentage();
  int batteryTimeRemaining = batteryMonitor.estimateTimeRemaining();
  
  if (isnan(temperature) || isnan(humidity)) {
    fancyLog.toSerial("Failed to read sensor", ERROR);
    setState(SYS_ERROR);
    return;
  }
  
  // Display with consistent decimal places
  fancyLog.toSerial("Temp: " + String(temperature, 1) + "°C, Humidity: " + String(humidity, 1) + "%");
  
  // Store data in buffer
  dataManager.storeReading(temperature, humidity, batteryVoltage, 
                          batteryPercentage, batteryTimeRemaining);
  
  // If buffer is full, send data
  if (dataManager.isBufferFull()) {
    dataManager.sendData();
  }
}

void SystemManager::checkBattery() {
  batteryMonitor.logStatus();
}

void SystemManager::checkForUpdates() {
  networkManager.checkForUpdates();
}

void SystemManager::updateDisplay() {
  switch (currentState) {
    case RUNNING:
      if (batteryMonitor.isLowBattery()) {
        displayManager.showNeutralFace(); // Use neutral face for low battery
      } else {
        displayManager.showHappyFace();
      }
      break;
    
    case SYS_ERROR:
    case CONNECTING:
      displayManager.showSadFace();
      break;
    
    case UPDATE_AVAILABLE:
      displayManager.showUpdateAvailable();
      break;
    
    case UPDATING:
      // Display is managed by the update process
      break;
    
    case INITIALIZING:
    default:
      displayManager.showNeutralFace();
      break;
  }
}
