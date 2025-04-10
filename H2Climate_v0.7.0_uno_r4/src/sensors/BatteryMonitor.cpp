#include "BatteryMonitor.h"
#include <Arduino.h>

BatteryMonitor::BatteryMonitor(Logger& logger)
    : logger(logger) {}

void BatteryMonitor::begin() {
    // Initialize battery monitoring pin
    pinMode(BATTERY_PIN, INPUT);
    
    // Take an initial reading
    float voltage = readVoltage();
    int percentage = readPercentage();
    int timeRemaining = estimateTimeRemaining();
    
    logger.log("Initial battery reading: " + String(voltage, 2) + "V (actual ~" + 
               String(voltage * VOLTAGE_TO_BATTERY, 1) + "V, " + 
               String(percentage) + "%) - Est. " + String(timeRemaining) + " minutes remaining");
               
    if (isLowBattery()) {
        logger.logWithBorder("WARNING: LOW BATTERY");
    }
}

float BatteryMonitor::readVoltage() {
    // Read the analog value from the battery pin
    int rawValue = analogRead(BATTERY_PIN);
    
    // Add some smoothing by taking multiple readings
    for (int i = 0; i < 10; i++) {
        rawValue = (rawValue + analogRead(BATTERY_PIN)) / 2;
        delay(1);
    }
    
    // Convert to voltage (Arduino UNO R4 has 10-bit ADC with 3.3V reference)
    float voltage = rawValue * (3.3 / 1023.0);
    
    // Log raw voltage for debugging
    if (rawValue > 100) { // Only log reasonable values
        logger.log("Battery ADC raw: " + String(rawValue) + ", Voltage: " + 
                  String(voltage, 3) + "V (actual ~" + 
                  String(voltage * VOLTAGE_TO_BATTERY, 1) + "V)");
    }
    
    return voltage;
}

int BatteryMonitor::readPercentage() {
    // Get current battery voltage
    float voltage = readVoltage();
    
    // Calculate percentage based on calibrated voltage range
    // Multiply by 1000 to maintain precision in the map function
    long percentageCalc = map(
        (long)(voltage * 1000), 
        (long)(CALIBRATED_EMPTY_VOLTAGE * 1000), 
        (long)(CALIBRATED_FULL_VOLTAGE * 1000), 
        0, 
        100
    );
    
    // Constrain to 0-100 range
    int percentage = constrain(percentageCalc, 0, 100);
    
    return percentage;
}

int BatteryMonitor::estimateTimeRemaining() {
    // Get current battery percentage
    int percentage = readPercentage();
    
    // Estimate time remaining based on percentage of full battery life
    int timeRemaining = (percentage * FULL_BATTERY_LIFE_MINUTES) / 100;
    
    return timeRemaining;
}

void BatteryMonitor::logStatus() {
    float voltage = readVoltage();
    int percentage = readPercentage();
    int timeRemaining = estimateTimeRemaining();
    
    logger.logWithBorder("Battery Status");
    logger.log("Measured voltage: " + String(voltage, 3) + "V");
    logger.log("Estimated actual voltage: " + String(voltage * VOLTAGE_TO_BATTERY, 1) + "V");
    logger.log("Charge: " + String(percentage) + "%");
    
    // Format the remaining time in hours and minutes
    int hours = timeRemaining / 60;
    int minutes = timeRemaining % 60;
    
    logger.log("Est. time remaining: " + String(hours) + "h " + String(minutes) + "m");
    
    // Show low battery warning if below threshold
    if (isLowBattery()) {
        logger.logWithBorder("WARNING: LOW BATTERY");
    }
}

bool BatteryMonitor::isLowBattery() {
    return readPercentage() < LOW_BATTERY_THRESHOLD;
} 