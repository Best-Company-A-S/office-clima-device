#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "../config/Config.h"
#include "../utils/Logger.h"
#include "DHT.h"

class SensorManager {
public:
    SensorManager(Logger& logger);
    void begin();
    float readTemperature();
    float readHumidity();
    bool isReady();
    
    // Making DHT instance public like in the original code
    DHT dht;

private:
    Logger& logger;
};

#endif // SENSOR_MANAGER_H 