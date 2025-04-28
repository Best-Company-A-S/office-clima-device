#include "Config.h"
#include "secrets.h"

// Network Configuration
const char* WIFI_SSID = WIFI_SSID_VALUE;
const char* WIFI_PASS = WIFI_PASS_VALUE;
const char* SERVER_URL = SERVER_URL_VALUE;
const int SERVER_PORT = SERVER_PORT_VALUE;

// API Routes
const char* API_REGISTER_ROUTE = API_REGISTER_ROUTE_VALUE;
const char* API_DATA_ROUTE = API_DATA_ROUTE_VALUE;

// Device Configuration
// DEVICE_ID is now handled by DeviceIdentifier class
const char* MODEL_TYPE = MODEL_TYPE_VALUE;
const char* FIRMWARE_VERSION = FIRMWARE_VERSION_VALUE;

// Timing Configuration
const unsigned long LOOP_INTERVAL = LOOP_INTERVAL_VALUE;
const unsigned long API_TIMEOUT = API_TIMEOUT_VALUE;
const unsigned long WIFI_TIMEOUT = WIFI_TIMEOUT_VALUE;
const int MAX_API_ATTEMPTS = MAX_API_ATTEMPTS_VALUE;
// CHECK_INTERVAL is defined directly in params.h
// BATTERY_LOG_INTERVAL is defined directly in params.h

// Animation Configuration
const int RETRY_ANIMATION_BLINKS = RETRY_ANIMATION_BLINKS_VALUE;
const int RETRY_ANIMATION_ON_TIME = RETRY_ANIMATION_ON_TIME_VALUE;
const int RETRY_ANIMATION_OFF_TIME = RETRY_ANIMATION_OFF_TIME_VALUE;

// Sensor Configuration
const int DHT22_PIN = DHT22_PIN_VALUE;

// Data Buffer Configuration is defined directly in params.h as a preprocessor define

// Battery Monitor Configuration is defined directly in params.h