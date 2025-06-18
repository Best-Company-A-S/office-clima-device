#include "SensorManager.h"

SensorManager::SensorManager(FancyLog& fancyLog)
    : fancyLog(fancyLog), dht(DHT22_PIN, DHTTYPE) {}

void SensorManager::begin() {
    initializeDHTSensor(); // Initialize the DHT sensor
	initializeCO2Sensor(); // Initialize the CO2 sensor
}

void SensorManager::testSensors() {
	fancyLog.toSerial("vvv [Testing sensors ] vvv");
    testDHTSensor(); // Test DHT sensor
	testCO2Sensor(); // Test CO2 sensor
	fancyLog.toSerial("^^^ [Testing complete] ^^^");
}

//¤======================¤
//| DHT sensor Functions |
//¤======================¤================================================================¤

void SensorManager::initializeDHTSensor() {
    dht.begin();
	fancyLog.toSerial("DHT sensor initialized");
}

void SensorManager::testDHTSensor() {
	fancyLog.toSerial("Testing DHT sensor");

	// uncomment if delay is needed for sensor stabilization
    //delay(2000); // Give the sensor time to stabilize

    // Attempt a test reading to verify the sensor is working
    float temp = readTemperature();
    float hum = readHumidity();

    if (isnan(temp) || isnan(hum)) {
        fancyLog.toSerial("DHT sensor test failed, but continuing anyway");
    } else {
        fancyLog.toSerial("DHT sensor test result: Temp=" + String(temp) + "°C, Humidity=" + String(hum) + "%");
    }
}

float SensorManager::readTemperature() {
    return dht.readTemperature();
}

float SensorManager::readHumidity() {
    return dht.readHumidity();
}

//¤======================¤
//| CO2 sensor Functions |
//¤======================¤================================================================¤

void SensorManager::initializeCO2Sensor() {
	// Simulated initialization for CO2 sensor as it didn't arrive in time
	fancyLog.toSerial("CO2 sensor initialized");
}

void SensorManager::testCO2Sensor() {
	fancyLog.toSerial("Testing CO2 sensor");
	fancyLog.toSerial("CO2 sensor test failed, but continuing anyway");
}