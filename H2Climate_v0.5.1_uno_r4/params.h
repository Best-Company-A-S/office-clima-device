// Device information
#define DEVICE_ID "6fe26f8eaf7e"
#define MODEL_TYPE "Arduino_UNO_R4_WiFi"
#define FIRMWARE_VERSION "v0.5.1"

// Server settings
#define SERVER_URL "10.106.186.200"
#define SERVER_PORT 3000
#define API_TIMEOUT 10000

// API endpoints
#define API_REGISTER_ROUTE "/api/device/register"
#define API_DATA_ROUTE "/api/devices/readings"

// DHT sensor settings
#define DHT22_PIN 2
#define DHTTYPE DHT22

// Timing parameters
#define LOOP_INTERVAL 60000      // Main loop interval (1 minute)
#define WIFI_TIMEOUT 20000       // WiFi connection timeout
#define MAX_API_ATTEMPTS 3       // Maximum number of API retry attempts
#define RETRY_DELAY 1000        // Delay between retries

// Animation settings
#define RETRY_ANIMATION_BLINKS 3
#define RETRY_ANIMATION_ON_TIME 200
#define RETRY_ANIMATION_OFF_TIME 200