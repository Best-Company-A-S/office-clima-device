#ifndef CONFIG_H
#define CONFIG_H

// Include Arduino libraries
#include <Arduino.h>
#include <TimeLib.h>
#include <WiFiS3.h>
#include "DHT.h"
#include "Arduino_LED_Matrix.h"
#include <ArduinoJson.h>
#include "ArduinoGraphics.h"
#include "../utils/DeviceIdentifier.h"

// Device Configuration
// DEVICE_ID is now retrieved dynamically from DeviceIdentifier class
extern const char* MODEL_TYPE;
extern const char* FIRMWARE_VERSION;
#define MODEL_TYPE_VALUE "Arduino_UNO_R4_WiFi"
#define FIRMWARE_VERSION_VALUE "V0.7.2"

// Network Configuration    
extern const char* WIFI_SSID;     // Define in secrets.h
extern const char* WIFI_PASS;     // Define in secrets.h
extern const char* SERVER_URL;
extern const int SERVER_PORT;
#define SERVER_URL_VALUE "10.106.187.92"
#define SERVER_PORT_VALUE 3000

// API Routes
extern const char* API_REGISTER_ROUTE;
extern const char* API_DATA_ROUTE;
#define API_REGISTER_ROUTE_VALUE "/api/device/register"
#define API_DATA_ROUTE_VALUE "/api/devices/readings"

// Timing Configuration
extern const unsigned long LOOP_INTERVAL;      // Main loop interval in milliseconds
extern const unsigned long API_TIMEOUT;        // API timeout in milliseconds
extern const unsigned long WIFI_TIMEOUT;       // WiFi connection timeout in milliseconds
extern const int MAX_API_ATTEMPTS;             // Maximum API connection attempts
#define LOOP_INTERVAL_VALUE 10000      // Main loop interval (10 seconds)
#define API_TIMEOUT_VALUE 10000        // API timeout in milliseconds
#define WIFI_TIMEOUT_VALUE 20000       // WiFi connection timeout
#define MAX_API_ATTEMPTS_VALUE 3       // Maximum API connection attempts

// Update Check Configuration
#define CHECK_INTERVAL 3600000UL // 1 hour in milliseconds - Update check interval

// Battery Log Configuration 
#define BATTERY_LOG_INTERVAL 10000 // Check battery every 10 seconds for debugging

// Animation Configuration
extern const int RETRY_ANIMATION_BLINKS;
extern const int RETRY_ANIMATION_ON_TIME;
extern const int RETRY_ANIMATION_OFF_TIME;
#define RETRY_ANIMATION_BLINKS_VALUE 3
#define RETRY_ANIMATION_ON_TIME_VALUE 500
#define RETRY_ANIMATION_OFF_TIME_VALUE 500

// Sensor Configuration
extern const int DHT22_PIN;       // Define the pin where DHT22 is connected
#define DHT22_PIN_VALUE 2
#define DHTTYPE DHT22

// Data Buffer Configuration
#define DATA_BUFFER_SIZE 1       // Number of readings to store before sending

// Battery Monitor Configuration
#define BATTERY_PIN A0           // Analog pin for battery monitoring
#define BATTERY_REFERENCE_VOLTAGE 3.3  // Reference voltage for ADC
#define BATTERY_FULL_VOLTAGE 4.2       // Full battery voltage
#define BATTERY_EMPTY_VOLTAGE 3.3      // Empty battery voltage
#define BATTERY_RESISTOR_RATIO 2.0     // Voltage divider ratio (if using a resistor divider)

#endif // CONFIG_H 