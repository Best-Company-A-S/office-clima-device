// Device details

#define DEVICE_ID "6fe26f8eaf7e" // second device // Test ID

#define MODEL_TYPE "Arduino_UNO_R4_WiFi"
#define FIRMWARE_VERSION "v0.4"

// Sensor details
#define DHT22_PIN 12
#define DHTTYPE DHT22

// Server details
// Using the Network IP shown in the Next.js console
#define SERVER_URL "10.106.186.200"

#define SERVER_PORT 3000
// Make sure these API routes match your Next.js API routes
#define API_DATA_ROUTE "/api/devices/readings"
#define API_REGISTER_ROUTE "/api/device/register"

// Timing settings
#define API_TIMEOUT   15000  // 15 seconds - timeout for API responses
#define WIFI_TIMEOUT  30000  // 30 seconds - timeout for WiFi connection
#define LOOP_INTERVAL 60000  // 60 seconds (1 minute) - interval between readings

// Retry settings
#define MAX_API_ATTEMPTS 3      // Maximum number of API retry attempts
#define MAX_SERVER_CONNECT_ATTEMPTS 3  // Maximum number of server connection attempts
#define RETRY_DELAY 1000        // Delay between retries in milliseconds
#define RETRY_ANIMATION_ON_TIME 300   // Time the face is shown during retry animation
#define RETRY_ANIMATION_OFF_TIME 200  // Time the face is off during retry animation
#define RETRY_ANIMATION_BLINKS 3      // Number of blinks in retry animation
