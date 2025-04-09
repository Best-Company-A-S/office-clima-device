#ifndef CONFIG_H
#define CONFIG_H

// Include Arduino libraries
#include <Arduino.h>
#include <TimeLib.h>
#include <WiFiS3.h>
#include "DHT.h"
#include "Arduino_LED_Matrix.h"
#include <ArduinoJson.h>

// Network Configuration    
extern const char* WIFI_SSID;     // Define in secrets.h
extern const char* WIFI_PASS;     // Define in secrets.h
extern const char* SERVER_URL;    // Define in params.h
extern const int SERVER_PORT;     // Define in params.h

// API Routes
extern const char* API_REGISTER_ROUTE;  // Define in params.h
extern const char* API_DATA_ROUTE;      // Define in params.h

// Device Configuration
extern const char* DEVICE_ID;     // Define in params.h
extern const char* MODEL_TYPE;    // Define in params.h
extern const char* FIRMWARE_VERSION; // Define in params.h

// Timing Configuration
extern const unsigned long LOOP_INTERVAL;      // 1 minute in milliseconds
extern const unsigned long API_TIMEOUT;        // 10 seconds in milliseconds
extern const unsigned long WIFI_TIMEOUT;       // 20 seconds in milliseconds
extern const int MAX_API_ATTEMPTS;

// Animation Configuration
extern const int RETRY_ANIMATION_BLINKS;
extern const int RETRY_ANIMATION_ON_TIME;
extern const int RETRY_ANIMATION_OFF_TIME;

// Sensor Configuration
extern const int DHT22_PIN;  // Define the pin where DHT22 is connected
#define DHTTYPE DHT22

// Update Check Configuration
#ifndef CHECK_INTERVAL
#define CHECK_INTERVAL 3600000UL  // 1 hour in milliseconds
#endif

#endif // CONFIG_H 