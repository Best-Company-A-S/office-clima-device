// Device details
#define DEVICE_ID "6fe26f8eaf7e" // Test ID
#define MODEL_TYPE "Arduino_UNO_R4_WiFi"
#define FIRMWARE_VERSION "v0.4"

// Sensor details
#define DHT22_PIN 12
#define DHTTYPE DHT22

// Server details
#define SERVER_URL "clima-app-blush-beta.vercel.app"
#define SERVER_PORT 443
#define API_DATA_ROUTE "/api/device/readings"
#define API_REGISTER_ROUTE "/api/device/register"

// Timing settings
#define API_TIMEOUT   15000 // 15 seconds
#define WIFI_TIMEOUT  20000 // 20 seconds
#define LOOP_INTERVAL 10000 // 10 seconds
