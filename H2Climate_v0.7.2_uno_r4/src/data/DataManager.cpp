#include "DataManager.h"
#include "../network/NetworkManager.h"

DataManager::DataManager(NetworkManager& network, FancyLog& log) 
  : networkManager(network), fancyLog(log), dataCount(0) {
}

void DataManager::storeReading(float temperature, float humidity,
                              float batteryVoltage, int batteryPercentage,
                              int batteryTimeRemaining) {
  // Check if there is space in the buffer                              
  if (dataCount < DATA_BUFFER_SIZE) {
    dataBuffer[dataCount] = {
      temperature,
      humidity,
      batteryVoltage,
      batteryPercentage,
      batteryTimeRemaining,
      now()  // Use Unix timestamp from TimeLib
    };
    dataCount++;
    
    fancyLog.toSerial("Stored reading in buffer slot " + String(dataCount), INFO);
  } else {
    fancyLog.toSerial("Data buffer is full, cannot store reading", WARNING);
  }
}

bool DataManager::sendData() {
  if (dataCount == 0) {
    fancyLog.toSerial("No data to send", WARNING);
    return false;
  }
  
  String jsonPayload = createJsonPayload();
  
  // Log the exact JSON format
  fancyLog.toSerial("JSON Format: " + jsonPayload);
  
  if (networkManager.sendHttpPostRequest(jsonPayload, API_DATA_ROUTE)) {
    fancyLog.toSerial("Data sent successfully", INFO);
    clearBuffer();
    return true;
  } else {
    fancyLog.toSerial("Failed to send data", ERROR);
    return false;
  }
}

bool DataManager::isBufferFull() const {
  return dataCount >= DATA_BUFFER_SIZE;
}

void DataManager::clearBuffer() {
  dataCount = 0;
  fancyLog.toSerial("Data buffer cleared", INFO);
}

int DataManager::getBufferCapacity() const {
  return DATA_BUFFER_SIZE;
}

int DataManager::getReadingCount() const {
  return dataCount;
}

String DataManager::createJsonPayload() const {
  // Get the values we're sending (currently just using the first entry)
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
  
  return sensorData;
}
