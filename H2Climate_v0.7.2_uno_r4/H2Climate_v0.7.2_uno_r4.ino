/*
 * H2Climate Device Firmware v0.7.2
 * For Arduino UNO R4 WiFi
 * 
 * This firmware provides temperature and humidity monitoring with
 * automatic firmware updates, LED matrix status display, and battery monitoring.
 * 
 * Refactored with modular architecture for improved maintainability.
 */

// Include Arduino core libraries first
#include <Arduino.h>
#include <TimeLib.h>

// Include project files in dependency order
#include "src/config/Config.h"
#include "src/utils/FancyLog.h"
#include "src/utils/DeviceIdentifier.h"
#include "src/display/DisplayManager.h"
#include "src/network/NetworkManager.h"
#include "src/sensors/SensorManager.h"
#include "src/sensors/BatteryMonitor.h"
#include "src/data/DataManager.h"
#include "src/system/SystemManager.h"

//¤=======================================================================================¤
//| TODO: Add sound sensor (Sound sensor is garbango so maybe not)                        |
//| TODO: Changeable settings                                                             |
//| TODO: Add warning triggers at certain temperatures and humidities                     |
//| TODO: Store more sensor data before sending a packet to reduce packet spam            |
//| TODO: Refactor BatteryMonitor class when a better solution is decided                 |
//¤=======================================================================================¤

// Global objects
FancyLog fancyLog;
DisplayManager display;
NetworkManager network(display, fancyLog);
SensorManager sensors(fancyLog);
BatteryMonitor battery(fancyLog);
DataManager dataManager(network, fancyLog);
SystemManager sysManager(display, network, sensors, battery, dataManager, fancyLog);

// Setup function runs once at startup
void setup() {
  // Initialize the system
  sysManager.begin();
}

// Main loop
void loop() {
  // Update the system
  sysManager.update();
  
  // Allow for some idle time to process background tasks
  delay(10);
}