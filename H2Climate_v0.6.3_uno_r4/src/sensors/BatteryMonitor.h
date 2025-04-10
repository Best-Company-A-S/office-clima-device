#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include "../utils/Logger.h"

class BatteryMonitor {
public:
    BatteryMonitor(Logger& logger);
    void begin();
    float readVoltage();
    int readPercentage();
    int estimateTimeRemaining(); // Returns estimated minutes remaining
    void logStatus();
    bool isLowBattery();
    
private:
    Logger& logger;
    const int BATTERY_PIN = A0; // Analog pin for battery voltage reading
    
    // These are the actual voltage values of a 9V battery
    const float BATTERY_MAX_VOLTAGE = 9.0; // 9V battery max voltage
    const float BATTERY_MIN_VOLTAGE = 7.0; // Cutoff voltage for 9V battery
    
    // These values should match what your voltage divider is showing
    const float CALIBRATED_FULL_VOLTAGE = 0.7; // For a full 9V battery, your readings show ~0.7V
    const float CALIBRATED_EMPTY_VOLTAGE = 0.5; // For an empty battery (estimate)
    
    // This ratio is for display purposes only - to show the estimated actual battery voltage
    // Based on your readings of ~0.7V corresponding to an actual 9V battery
    const float VOLTAGE_TO_BATTERY = 13.0; // Multiplier to convert measured voltage to actual battery voltage
    
    // Estimated battery life in minutes at full charge 
    const int FULL_BATTERY_LIFE_MINUTES = 480; // 8 hours for a typical 9V battery
    const int LOW_BATTERY_THRESHOLD = 20; // Low battery warning threshold (percentage)
};

#endif // BATTERY_MONITOR_H 