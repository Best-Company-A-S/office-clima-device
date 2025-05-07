#ifndef CONFIG_H
#define CONFIG_H

// Include Arduino libraries
#include <Arduino.h>
#include <TimeLib.h>
#include <WiFiS3.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <Arduino_LED_Matrix.h>
#include <ArduinoGraphics.h>
#include "../utils/DeviceIdentifier.h"
#include "../config/secrets.h"

//¤======================¤
//| Device Configuration |
//¤======================¤================================================================¤
// DEVICE_ID is retrieved dynamically from DeviceIdentifier class
constexpr const char* MODEL_TYPE = "Arduino_UNO_R4_WiFi";
constexpr const char* FIRMWARE_VERSION = "V0.8.0";

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
constexpr const int BATTERY_PIN = A0; // Analog pin for battery voltage reading

// These are the actual voltage values of a 9V battery
constexpr const float BATTERY_MAX_VOLTAGE = 9.0; // 9V battery max voltage
constexpr const float BATTERY_MIN_VOLTAGE = 7.0; // Cutoff voltage for 9V battery

// These values should match what your voltage divider is showing
constexpr const float CALIBRATED_FULL_VOLTAGE = 0.7; // For a full 9V battery, your readings show ~0.7V
constexpr const float CALIBRATED_EMPTY_VOLTAGE = 0.5; // For an empty battery (estimate)

// This ratio is for display purposes only - to show the estimated actual battery voltage
// Based on your readings of ~0.7V corresponding to an actual 9V battery
constexpr const float VOLTAGE_TO_BATTERY = 13.0; // Multiplier to convert measured voltage to actual battery voltage

// Estimated battery life in minutes at full charge
constexpr const int FULL_BATTERY_LIFE_MINUTES = 480; // 8 hours for a typical 9V battery
constexpr const int LOW_BATTERY_THRESHOLD = 20; // Low battery warning threshold (percentage)

#endif // CONFIG_H 