#include "SensorManager.h"

SensorManager::SensorManager(Logger& logger)
    : logger(logger), dht(DHT22_PIN, DHTTYPE) {}

void SensorManager::begin() {
    logger.logWithBorder("Initializing DHT sensor");
    dht.begin();
    
    // Give the sensor time to stabilize - this is critical
    // Using the exact same delay as in the original code
    delay(2000);
    
    // Attempt an initial reading to verify the sensor is working
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    
    if (isnan(temp) || isnan(hum)) {
        logger.logWithBorder("WARNING: Initial sensor reading failed, but continuing anyway");
    } else {
        logger.log("Initial reading: Temp=" + String(temp) + "°C, Humidity=" + String(hum) + "%");
    }
    
    logger.logWithBorder("DHT sensor initialized");
}

float SensorManager::readTemperature() {
    // Direct call to DHT sensor exactly like original code
    return dht.readTemperature();
}

float SensorManager::readHumidity() {
    // Direct call to DHT sensor exactly like original code
    return dht.readHumidity();
}

bool SensorManager::isReady() {
    // Always return true like in the original code
    // The original code didn't have a specific isReady check
    return true;
} 