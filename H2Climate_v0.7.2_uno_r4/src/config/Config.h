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
#include "../config/secrets.h"

//¤======================¤
//| Device Configuration |
//¤======================¤================================================================¤
// DEVICE_ID is retrieved dynamically from DeviceIdentifier class
constexpr const char* MODEL_TYPE = "Arduino_UNO_R4_WiFi";
constexpr const char* FIRMWARE_VERSION = "V0.7.2";

//¤=======================¤
//| Network Configuration |
//¤=======================¤===============================================================¤
constexpr const char* SERVER_URL = "10.106.187.92";
constexpr const int SERVER_PORT = 3000;

//¤============¤
//| API Routes |
//¤============¤==========================================================================¤
constexpr const char* API_REGISTER_ROUTE = "/api/device/register";
constexpr const char* API_DATA_ROUTE = "/api/devices/readings";

//¤======================¤
//| Timing Configuration |
//¤======================¤================================================================¤
// Main loop interval in milliseconds (10 seconds)
constexpr const unsigned long LOOP_INTERVAL = 10000;
// API request timeout in milliseconds
constexpr const unsigned long API_TIMEOUT = 10000;
// WiFi connection timeout in milliseconds
constexpr const unsigned long WIFI_TIMEOUT = 20000;
// Maximum number of API connection attempts before giving up
constexpr const int MAX_API_ATTEMPTS = 3;
// Update check interval (1 hour in milliseconds)
constexpr const unsigned long CHECK_INTERVAL = 3600000UL;
// Battery status logging interval (10 seconds)
constexpr const unsigned long BATTERY_LOG_INTERVAL = 10000;

//¤=========================¤
//| Animation Configuration |
//¤=========================¤=============================================================¤
// Number of times to blink during retry animation
constexpr const int RETRY_ANIMATION_BLINKS = 3;
// Time in milliseconds that LED is on during retry animation
constexpr const int RETRY_ANIMATION_ON_TIME = 500;
// Time in milliseconds that LED is off during retry animation
constexpr const int RETRY_ANIMATION_OFF_TIME = 500;

//¤======================¤
//| Sensor Configuration |
//¤======================¤================================================================¤
// Define the pin where DHT22 is connected
constexpr const int DHT22_PIN = 2;
// DHT sensor type definition
#define DHTTYPE DHT22

//¤===========================¤
//| Data Buffer Configuration |
//¤===========================¤===========================================================¤
// Number of readings to store before sending to server
constexpr const int DATA_BUFFER_SIZE = 1;

//¤===============================¤
//| Battery Monitor Configuration |
//¤===============================¤=======================================================¤
// Analog pin for battery monitoring
constexpr const int BATTERY_PIN = A0;
// Reference voltage for ADC in volts
constexpr const float BATTERY_REFERENCE_VOLTAGE = 3.3;
// Full battery voltage in volts
constexpr const float BATTERY_FULL_VOLTAGE = 4.2;
// Empty battery voltage in volts
constexpr const float BATTERY_EMPTY_VOLTAGE = 3.3;
// Voltage divider ratio (if using a resistor divider)
constexpr const float BATTERY_RESISTOR_RATIO = 2.0;

#endif // CONFIG_H 