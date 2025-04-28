#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "../config/Config.h"
#include "../utils/FancyLog.h"
#include "../utils/DeviceIdentifier.h"
#include "../network/NetworkManager.h"

// Data structure for sensor readings
struct SensorData {
  float temperature;
  float humidity;
  float batteryVoltage;
  int batteryPercentage;
  int batteryTimeRemaining;
  unsigned long timestamp;
};

class DataManager {
public:
  DataManager(NetworkManager& network, FancyLog& log);
  
  // Store a new sensor reading
  void storeReading(float temperature, float humidity, 
                   float batteryVoltage, int batteryPercentage, 
                   int batteryTimeRemaining);

  bool sendData(); // Send stored data to server
  bool isBufferFull() const; // Check if buffer is full
  void clearBuffer(); // Clear data buffer
  int getBufferCapacity() const; // Get buffer capacity
  int getReadingCount() const; // Get current number of readings
private:
  NetworkManager& networkManager;
  FancyLog& fancyLog;
  
  static const int DATA_BUFFER_SIZE = 1;  // Number of readings to store before sending
  int dataCount;
  SensorData dataBuffer[DATA_BUFFER_SIZE];

  String createJsonPayload() const; // Create JSON payload from stored data
};

#endif // DATA_MANAGER_H
