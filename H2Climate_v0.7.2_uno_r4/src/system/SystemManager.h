#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include "../config/Config.h"
#include "../utils/FancyLog.h"

// Forward declarations to avoid circular dependencies
class DisplayManager;
class NetworkManager;
class SensorManager;
class BatteryMonitor;
class DataManager;

// System states
enum SystemState {
  INITIALIZING,
  CONNECTING,
  RUNNING,
  SYS_ERROR,  // Renamed from ERROR to avoid conflict with LogLevel
  UPDATE_AVAILABLE,
  UPDATING
};

class SystemManager {
public:
  SystemManager(DisplayManager& display, NetworkManager& network, 
                SensorManager& sensors, BatteryMonitor& battery,
                DataManager& data, FancyLog& log);
  
  // Initialize the system
  void begin();
  
  // Main system update function, to be called in loop()
  void update();
  
  // Get current system state
  SystemState getState() const;
  
  // Set system state
  void setState(SystemState newState);

private:
  DisplayManager& displayManager;
  NetworkManager& networkManager;
  SensorManager& sensorManager;
  BatteryMonitor& batteryMonitor;
  DataManager& dataManager;
  FancyLog& fancyLog;
  
  // System state
  SystemState currentState;
  
  // Timing variables
  unsigned long previousSensorReadMillis;
  unsigned long previousUpdateCheckMillis;
  unsigned long previousBatteryLogMillis;
  
  // Timing constants (using direct values to avoid non-constant expression error)
  static const unsigned long SENSOR_READ_INTERVAL = 10000;  // Same as LOOP_INTERVAL_VALUE
  static const unsigned long UPDATE_CHECK_INTERVAL = 3600000UL;  // Same as CHECK_INTERVAL
  static const unsigned long BATTERY_LOG_INTERVAL = 10000;
  
  // Internal methods
  void registerDevice();
  void readSensors();
  void checkBattery();
  void checkForUpdates();
  void updateDisplay();
};

#endif // SYSTEM_MANAGER_H
