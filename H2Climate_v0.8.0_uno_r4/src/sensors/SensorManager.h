#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "../config/Config.h"
//#include "../utils/FancyLog.h"
//#include "DHT.h"

class SensorManager {
  public:
    SensorManager(FancyLog& fancyLog);
    void begin();
    float readTemperature();
    float readHumidity();
    DHT dht; // Public DHT instance for legacy reasons

  private:
    FancyLog& fancyLog;
};

#endif // SENSOR_MANAGER_H 