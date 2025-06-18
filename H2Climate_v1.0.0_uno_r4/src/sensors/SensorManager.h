#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "../config/Config.h"
#include "../utils/FancyLog.h"

class SensorManager {
  public:
    SensorManager(FancyLog& fancyLog);
    void begin();
	void testSensors();
    float readTemperature();
    float readHumidity();

  private:
	void initializeDHTSensor();
	void initializeCO2Sensor();
	void testDHTSensor();
	void testCO2Sensor();
    DHT dht;
    FancyLog& fancyLog;
};

#endif // SENSOR_MANAGER_H 