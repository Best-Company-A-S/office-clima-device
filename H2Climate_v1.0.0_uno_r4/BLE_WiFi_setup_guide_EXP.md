# H2Climate Bluetooth WiFi Setup Guide

This guide will help you configure your H2Climate device's WiFi connection using a smartphone or tablet via Bluetooth. This feature allows you to provide WiFi credentials to your device without needing to modify the code or having a pre-existing WiFi connection.

## Requirements

- A smartphone or tablet with Bluetooth support
- A BLE (Bluetooth Low Energy) scanning app such as:
  - **nRF Connect** (recommended, available for [Android](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp) and [iOS](https://apps.apple.com/us/app/nrf-connect-for-mobile/id1054362403))
  - **LightBlue** ([Android](https://play.google.com/store/apps/details?id=com.punchthrough.lightblueexplorer) and [iOS](https://apps.apple.com/us/app/lightblue/id557428110))
- Your H2Climate device powered on

## Step-by-Step WiFi Configuration Process

### 1. Power On Your Device

Power on your H2Climate device. Upon startup, if no WiFi credentials are stored or if the stored credentials fail to connect, the device will automatically enter configuration mode, indicated by an alternating neutral/sad face on the LED matrix.

### 2. Scan for BLE Devices

- Open your BLE scanning app (e.g., nRF Connect)
- Enable Bluetooth on your smartphone if prompted
- Scan for available devices
- Look for a device named **"H2Climate-Arduino_UNO_R4_WiFi"**

### 3. Connect to Your H2Climate Device

- Tap on the H2Climate device in the scanning list
- Connect to the device
- Once connected, you should see available services

### 4. Configure WiFi Credentials

In your BLE app, you'll need to find and interact with the WiFi Configuration Service:

- Locate the service with UUID: **19B10000-E8F2-537E-4F6C-D104768A1214**
- Within this service, you'll find three characteristics:

#### For nRF Connect:

1. **SSID Characteristic** (UUID: 19B10001-E8F2-537E-4F6C-D104768A1214)
   - Tap on this characteristic
   - Select "Write"
   - Enter your WiFi network name (SSID) in text format
   - Confirm/Send

2. **Password Characteristic** (UUID: 19B10002-E8F2-537E-4F6C-D104768A1214)
   - Tap on this characteristic
   - Select "Write"
   - Enter your WiFi password in text format
   - Confirm/Send

3. **Confirm Characteristic** (UUID: 19B10003-E8F2-537E-4F6C-D104768A1214)
   - After entering both SSID and password
   - Tap on this characteristic
   - Select "Write"
   - Enter "01" or "true" (this confirms and submits your credentials)
   - Confirm/Send

### 5. Wait for Connection

After sending the confirmation:

- The H2Climate device will attempt to connect to your WiFi network with the provided credentials
- Watch the device's LED matrix display for feedback:
  - Neutral face: Attempting to connect
  - Happy face: Successfully connected
  - Sad face: Connection failed

### 6. Verify Connection

- If connection is successful, the device will save the credentials to its memory
- You can verify the connection by checking the device's serial monitor output (if connected to a computer)
- The device will also display its IP address via serial output
- You can access the device's web interface by navigating to: `http://[device-ip]:80` in a web browser

## Troubleshooting

1. **Device Not Appearing in BLE Scan**
   - Ensure the device is powered on
   - Verify the device is in configuration mode (alternating neutral/sad face)
   - Try moving closer to the device
   - Restart your smartphone's Bluetooth

2. **Cannot Connect to the Device**
   - Close and reopen the BLE app
   - Restart the H2Climate device
   - Ensure your smartphone's Bluetooth is functioning properly

3. **WiFi Connection Fails**
   - Verify SSID and password are correct (case-sensitive)
   - Ensure your WiFi network is operational
   - Check if your WiFi network requires special authentication
   - If connection fails, the device will remain in configuration mode, allowing you to try again

4. **Configuration Times Out**
   - The device will exit configuration mode after 5 minutes if no successful connection occurs
   - To re-enter configuration mode, power cycle the device while holding the reset button

## Notes

- Your WiFi credentials will be stored in the device's EEPROM memory and will persist between power cycles
- For security, the Bluetooth service is only active during configuration mode
- If you need to change WiFi networks, you can:
  1. Access the device's web interface and update settings there, or
  2. Reset the device and re-enter configuration mode

By following this guide, you should be able to quickly configure your H2Climate device to connect to your WiFi network without needing to modify or upload any code.
