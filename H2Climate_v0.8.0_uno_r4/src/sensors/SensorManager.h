#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "../config/Config.h"
#include "../utils/FancyLog.h"

class SensorManager {
  public:
    SensorManager(FancyLog& fancyLog);
    void begin();
    float readTemperature();
    float readHumidity();

  private:
    DHT dht;
    FancyLog& fancyLog;
};

#endif // SENSOR_MANAGER_H 