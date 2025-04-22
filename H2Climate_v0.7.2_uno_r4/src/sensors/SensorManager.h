#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "../config/Config.h"
#include "../utils/FancyLog.h"
#include "DHT.h"

class SensorManager {
public:
    SensorManager(FancyLog& fancyLog);
    void begin();
    float readTemperature();
    float readHumidity();
    bool isReady();
    
    // Making DHT instance public like in the original code
    DHT dht;

private:
    FancyLog& fancyLog;
};

#endif // SENSOR_MANAGER_H 