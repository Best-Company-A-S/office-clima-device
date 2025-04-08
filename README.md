# Office Climate Monitor (H2Climate)

<div align="center">
  <!-- Image will be added later -->
  <p><em>Monitor and optimize your office environment with real-time temperature and humidity tracking</em></p>
</div>

## 📊 Overview

H2Climate is an Arduino-based IoT device that monitors environmental conditions in office spaces. It collects temperature and humidity data and sends it to a server for analysis and visualization. The device features a built-in LED matrix that displays status information using simple emoji-like faces.

### Key Features

- 🌡️ Real-time temperature monitoring
- 💧 Humidity tracking
- 🔄 Automatic data synchronization
- 📶 WiFi connectivity
- 🕰️ NTP time synchronization
- 😊 Visual status indicators using LED matrix
- 🔋 Low-power operation (planned)

## 📐 System Architecture

```mermaid
flowchart LR
    A[H2Climate Device\nArduino UNO R4 WiFi] <-->|WiFi\nHTTP POST/JSON| B[Next.js Server\nData Storage & Visualization]

    style A fill:#b3e0ff,stroke:#0066cc,stroke-width:2px
    style B fill:#ffe6cc,stroke:#ff9933,stroke-width:2px
```

## 🛠️ Hardware Components

- Arduino UNO R4 WiFi
- DHT22 Temperature & Humidity Sensor
- LED Matrix (built into UNO R4)
- Power supply

### Hardware Connections

```mermaid
graph TD
    A[Arduino UNO R4 WiFi] --- B[Built-in LED Matrix]
    A -- Pin 12 --> C[DHT22 Sensor]
    A -- 5V --> C
    A -- GND --> C
    A -- USB/Power Port --> D[Power Supply]

    style A fill:#d4f0f0,stroke:#2E8BC0
    style B fill:#FFE6E6,stroke:#DA4167
    style C fill:#d8f8e1,stroke:#4CAF50
    style D fill:#fff9db,stroke:#FF9800
```

## 📦 Software Dependencies

- ArduinoJson
- TimeLib
- WiFiS3
- DHT sensor library
- Arduino_LED_Matrix

## 🔧 Installation & Setup

### Hardware Setup

1. Connect the DHT22 sensor to pin 12 on the Arduino UNO R4
2. Power the Arduino via USB or external power supply

### Software Setup

1. Clone this repository:

   ```bash
   git clone https://github.com/yourusername/office-clima-device.git
   cd office-clima-device
   ```

2. Create a `secrets.h` file in the project directory with your WiFi credentials:

   ```cpp
   #define WIFI_SSID "your_wifi_ssid"
   #define WIFI_PASS "your_wifi_password"
   ```

3. Modify `params.h` with your specific device settings:

   ```cpp
   #define DEVICE_ID "your_unique_device_id"
   #define SERVER_URL "your_server_url_or_ip"
   #define SERVER_PORT 3000
   ```

4. Upload the code to your Arduino using the Arduino IDE or PlatformIO

## 📈 Data Flow

```mermaid
sequenceDiagram
    participant Device as H2Climate Device
    participant Server as Next.js Server
    participant Database as Database
    participant UI as Web Dashboard

    Device->>Device: Collect temperature & humidity data
    Device->>Server: Send JSON data via HTTP POST
    Server->>Database: Store sensor readings
    Server->>Device: Send acknowledgment
    UI->>Server: Request data
    Server->>Database: Query data
    Database->>Server: Return data
    Server->>UI: Display visualizations

    Note over Device: LED Matrix shows status:<br/>😊 Connected<br/>😐 Setting up<br/>😟 Error
```

## 🚀 Version History

| Version | Features                                                       |
| ------- | -------------------------------------------------------------- |
| v0.1    | Basic temperature and humidity monitoring                      |
| v0.2    | Added WiFi connectivity and data transmission                  |
| v0.3    | Implemented NTP time synchronization                           |
| v0.4    | Added LED matrix status indicators and improved error handling |

## 📝 Future Improvements

```mermaid
mindmap
  root((H2Climate))
    Monitoring
      Battery logging
      Sound level monitoring
    Configuration
      Web interface settings
      Alert triggers
    Identification
      MAC address based IDs
    Optimization
      Data buffering
      Power management
```

## 📊 Web Dashboard

<!-- Dashboard image will be added later -->

The companion web dashboard displays:

- Current temperature and humidity readings
- Historical data trends
- Device status and connectivity information
- Alert notifications for out-of-range conditions

### Sample Temperature & Humidity Trends

```mermaid
%%{init: {'theme': 'forest'}}%%
xychart-beta
    title "Office Climate Data (24 Hours)"
    x-axis [9AM, 12PM, 3PM, 6PM, 9PM, 12AM, 3AM, 6AM]
    y-axis "Temperature (°C)" 18 --> 28
    y-axis "Humidity (%)" 30 --> 70
    line [22, 23.5, 24.2, 24.8, 23.1, 22.5, 21.8, 21.2]
    line [42, 44, 45, 43, 48, 52, 55, 50]
```

## 📡 API Endpoints

The device communicates with the following API endpoints:

- `/api/devices/readings` - POST endpoint for sending sensor data
- `/api/device/register` - POST endpoint for device registration

## 🔌 Communication Protocol

Data is sent using simple JSON packets:

```json
{
  "deviceId": "6fe26f8eaf1e",
  "temperature": 23.5,
  "humidity": 45.2,
  "timestamp": 1649276543
}
```

## ⚙️ Troubleshooting

- **Device shows sad face**: Check WiFi connection or sensor wiring
- **No data on server**: Verify server URL and port in params.h
- **Incorrect timestamps**: Check NTP server configuration

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 👥 Contributors

- [Your Name](https://github.com/yourusername)

## 🙏 Acknowledgements

- [Arduino Team](https://www.arduino.cc/) for the excellent hardware and libraries
- [Next.js](https://nextjs.org/) for the server framework
