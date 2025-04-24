# H2Climate Device Firmware

A firmware system for Arduino UNO R4 WiFi that monitors climate conditions (temperature and humidity) and transmits data to a server.

## Version History

- **V0.7.3**: Refactored codebase into modular architecture for improved maintainability
- **V0.7.2**: Added battery monitoring and improved status display
- **V0.7.1**: Implemented OTA update functionality
- **V0.6.x**: Added network connectivity and data transmission features
- **V0.5.x**: Implemented sensor reading and basic display functionality
- **V0.4.x**: Initial development versions

## Project Structure

The firmware has been organized into the following modules:

### Core Components

- **SystemManager**: Manages the overall system state and operation
- **DataManager**: Handles sensor data collection, storage, and transmission
- **NetworkManager**: Manages WiFi connectivity and API communications
- **DisplayManager**: Controls the LED matrix display
- **SensorManager**: Interfaces with temperature and humidity sensors
- **BatteryMonitor**: Monitors battery voltage and estimates remaining time

### Utilities

- **FancyLog**: Provides logging functionalities
- **DeviceIdentifier**: Generates and manages unique device identifiers

### Configuration

- **Config.h**: Main configuration header with external declarations
- **params.h**: Configuration parameters
- **secrets.h**: WiFi credentials (not tracked in Git for security)

## Setup Instructions

1. Create a `src/config/secrets.h` file with the following contents:
   ```cpp
   #ifndef SECRETS_H
   #define SECRETS_H
   
   // WiFi Credentials - replace with your network information
   #define WIFI_SSID_VALUE "your_wifi_ssid"
   #define WIFI_PASS_VALUE "your_wifi_password"
   
   #endif // SECRETS_H
   ```

2. Update server settings in `src/config/params.h` if needed
3. Upload the firmware to your Arduino UNO R4 WiFi
4. Monitor the serial console at 9600 baud to verify operation

## Features

- Temperature and humidity monitoring
- Battery level monitoring
- Data transmission to server
- Over-the-air (OTA) firmware updates
- Status display via LED matrix
