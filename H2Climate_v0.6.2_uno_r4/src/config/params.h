#ifndef PARAMS_H
#define PARAMS_H

// Device Configuration
#define DEVICE_ID_VALUE "6fe26f8eaf1e"
#define MODEL_TYPE_VALUE "Arduino_UNO_R4_WiFi"
#define FIRMWARE_VERSION_VALUE "V0.6.2"

// Server Configuration
#define SERVER_URL_VALUE "10.106.187.35"
#define SERVER_PORT_VALUE 3000
#define API_TIMEOUT_VALUE 10000
#define API_REGISTER_ROUTE_VALUE "/api/device/register"
#define API_DATA_ROUTE_VALUE "/api/devices/readings"
#define MAX_API_ATTEMPTS_VALUE 3

// Sensor Configuration
#define DHT22_PIN_VALUE 2

// Timing Configuration
#define LOOP_INTERVAL_VALUE 10000      // Main loop interval (10 seconds)
#define WIFI_TIMEOUT_VALUE 20000       // WiFi connection timeout

// Animation Configuration
#define RETRY_ANIMATION_BLINKS_VALUE 3
#define RETRY_ANIMATION_ON_TIME_VALUE 500
#define RETRY_ANIMATION_OFF_TIME_VALUE 500

#endif // PARAMS_H