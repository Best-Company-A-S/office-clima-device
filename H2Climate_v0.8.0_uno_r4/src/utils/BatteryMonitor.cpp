#include "BatteryMonitor.h"

BatteryMonitor::BatteryMonitor(FancyLog& fancyLog)
    : fancyLog(fancyLog) {}

//¤=======================================================================================¤

void BatteryMonitor::begin() {
    // Change to 14-bit resolution
	analogReadResolution(14);

    // Initialize battery monitoring pin
    pinMode(BATTERY_PIN, INPUT);
    
    // Take an initial reading
    logStatus();
}

//¤=======================================================================================¤

void BatteryMonitor::logStatus() {
    // Log the current battery status
    float voltage = readVoltage();
    int percentage = readPercentage(voltage);
    int timeRemaining = estimateTimeRemaining(percentage);

    fancyLog.toSerial("Battery reading: " +
			   String(voltage, 2) + "V (actual ~" +
               String(voltage * VOLTAGE_TO_BATTERY, 1) + "V, " +
               String(percentage) + "%) - Est. " +
			   String(timeRemaining) + " minutes remaining", INFO);

    // Show low battery warning if below threshold
    if (isLowBattery()) {
        fancyLog.toSerial("LOW BATTERY", WARNING);
    }
}

//¤=======================================================================================¤

float BatteryMonitor::readVoltage() {
    // Read the analog value from the battery pin
    int rawValue = analogRead(BATTERY_PIN);
    
    // Add some smoothing by taking multiple readings
    for (int i = 0; i < 10; i++) {
        rawValue = (rawValue + analogRead(BATTERY_PIN)) / 2;
        delay(1);
    }

	// Convert to voltage (The ADC is set to 14-bit resolution with 3.3V reference)
    float voltage = rawValue * (3.3 / 16383.0);

    return voltage;
}

//¤=======================================================================================¤

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

// Overloaded function to read percentage with voltage parameter
int BatteryMonitor::readPercentage(float voltage) {
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

//¤=======================================================================================¤

int BatteryMonitor::estimateTimeRemaining() {
    // Get current battery percentage
    int percentage = readPercentage();
    
    // Estimate time remaining based on percentage of full battery life
    int timeRemaining = (percentage * FULL_BATTERY_LIFE_MINUTES) / 100;
    
    return timeRemaining;
}

// Overloaded function to estimate time remaining with percentage parameter
int BatteryMonitor::estimateTimeRemaining(int percentage) {
    // Estimate time remaining based on percentage of full battery life
    int timeRemaining = (percentage * FULL_BATTERY_LIFE_MINUTES) / 100;

    return timeRemaining;
}

//¤=======================================================================================¤

bool BatteryMonitor::isLowBattery() {
    return readPercentage() < LOW_BATTERY_THRESHOLD;
}
