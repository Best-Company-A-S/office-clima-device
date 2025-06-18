#include "BluetoothManager.h"

// UUID for our WiFi Configuration Service
#define WIFI_CONFIG_SERVICE_UUID "19B10000-E8F2-537E-4F6C-D104768A1214"
#define WIFI_SSID_CHARACTERISTIC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"
#define WIFI_PASSWORD_CHARACTERISTIC_UUID "19B10002-E8F2-537E-4F6C-D104768A1214"
#define WIFI_CONFIRM_CHARACTERISTIC_UUID "19B10003-E8F2-537E-4F6C-D104768A1214"

// Initialize the static instance pointer
BluetoothManager* BluetoothManager::_instance = nullptr;

BluetoothManager::BluetoothManager(FancyLog& log) :
    logger(log),
    wifiConfigService(WIFI_CONFIG_SERVICE_UUID),
    ssidCharacteristic(WIFI_SSID_CHARACTERISTIC_UUID, BLERead | BLEWrite, 33),
    passwordCharacteristic(WIFI_PASSWORD_CHARACTERISTIC_UUID, BLERead | BLEWrite, 64),
    confirmCharacteristic(WIFI_CONFIRM_CHARACTERISTIC_UUID, BLERead | BLEWrite),
    credentialsReceived(false) {

    ssidBuffer[0] = '\0';
    passwordBuffer[0] = '\0';

    // Store instance pointer for static callback
    _instance = this;
}

bool BluetoothManager::begin() {
    // Initialize BLE hardware
    if (!BLE.begin()) {
        logger.toSerial("Failed to initialize Bluetooth!", ERROR);
        return false;
    }

    // Set the local name for the device (visible in BLE scanning)
    String deviceName = "H2Climate-" + String(MODEL_TYPE);
    BLE.setLocalName(deviceName.c_str());
    BLE.setAdvertisedService(wifiConfigService);

    setupBLEService();

    // Start advertising
    BLE.advertise();
    logger.toSerial("Bluetooth initialized - waiting for connection to configure WiFi", INFO);
    return true;
}

void BluetoothManager::setupBLEService() {
    // Set initial values for the characteristics
    ssidCharacteristic.setValue("");
    passwordCharacteristic.setValue("");
    confirmCharacteristic.setValue(false);

    // Add characteristics to the service
    wifiConfigService.addCharacteristic(ssidCharacteristic);
    wifiConfigService.addCharacteristic(passwordCharacteristic);
    wifiConfigService.addCharacteristic(confirmCharacteristic);

    // Add the service to the BLE device
    BLE.addService(wifiConfigService);

    // Set up the event handler for the confirm characteristic using a static callback function
    confirmCharacteristic.setEventHandler(BLEWritten, confirmCharacteristicWrittenCallback);
}

// Static callback function that BLE library can call
void BluetoothManager::confirmCharacteristicWrittenCallback(BLEDevice central, BLECharacteristic characteristic) {
    if (_instance) {
        _instance->handleConfirmCharacteristicWritten();
    }
}

// Instance method to handle the actual logic
void BluetoothManager::handleConfirmCharacteristicWritten() {
    if (confirmCharacteristic.value()) {
        // Copy SSID and password to buffers
        strncpy(this->ssidBuffer, this->ssidCharacteristic.value().c_str(), sizeof(this->ssidBuffer) - 1);
        this->ssidBuffer[sizeof(this->ssidBuffer) - 1] = '\0';  // Ensure null termination

        strncpy(this->passwordBuffer, this->passwordCharacteristic.value().c_str(), sizeof(this->passwordBuffer) - 1);
        this->passwordBuffer[sizeof(this->passwordBuffer) - 1] = '\0';  // Ensure null termination

        this->credentialsReceived = true;
        this->logger.toSerial("WiFi credentials received via Bluetooth!", INFO);
    }
}

void BluetoothManager::poll() {
    // Listen for BLE peripherals
    BLEDevice central = BLE.central();

    if (central) {
        // Central device connected
        logger.toSerial("Connected to central device: " + String(central.address()), INFO);

        // While the central is connected
        while (central.connected()) {
            // Check if we've received credentials
            if (credentialsReceived) {
                // We'll handle this in the main loop
                break;
            }
            delay(10);
        }

        // Disconnected
        logger.toSerial("Disconnected from central device", INFO);
    }
}

bool BluetoothManager::hasWifiCredentials() {
    return credentialsReceived;
}

void BluetoothManager::getWifiCredentials(char* ssid, char* password) {
    if (credentialsReceived) {
        strcpy(ssid, ssidBuffer);
        strcpy(password, passwordBuffer);
    }
}

void BluetoothManager::resetCredentialFlag() {
    credentialsReceived = false;
}
