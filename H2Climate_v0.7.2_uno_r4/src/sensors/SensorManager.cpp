#include "SensorManager.h"

SensorManager::SensorManager(FancyLog& fancyLog)
    : fancyLog(fancyLog), dht(DHT22_PIN, DHTTYPE) {}

void SensorManager::begin() {
    fancyLog.toSerial("Initializing DHT sensor", INFO);
    dht.begin();
    
    // Give the sensor time to stabilize - this is critical
    // Using the exact same delay as in the original code
    delay(2000);
    
    // Attempt an initial reading to verify the sensor is working
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    
    if (isnan(temp) || isnan(hum)) {
        fancyLog.toSerial("Initial sensor reading failed, but continuing anyway", WARNING);
    } else {
        fancyLog.toSerial("Initial reading: Temp=" + String(temp) + "°C, Humidity=" + String(hum) + "%", INFO);
    }
    
    fancyLog.toSerial("DHT sensor initialized", INFO);
}

float SensorManager::readTemperature() {
    // Direct call to DHT sensor exactly like original code
    return dht.readTemperature();
}

float SensorManager::readHumidity() {
    // Direct call to DHT sensor exactly like original code
    return dht.readHumidity();
}
