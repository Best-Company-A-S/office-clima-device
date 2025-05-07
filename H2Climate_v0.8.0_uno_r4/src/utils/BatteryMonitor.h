#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include "../config/Config.h"
#include "../utils/FancyLog.h"

class BatteryMonitor {
  public:
    BatteryMonitor(FancyLog& fancyLog);
    void begin();
	void logStatus();
    float readVoltage(); // Returns estimated voltage
	int readPercentage(); // If voltage is not provided, it will read the current voltage
    int readPercentage(float voltage); // Returns estimated percentage
	int estimateTimeRemaining(); // If percentage is not provided, it will read the current percentage
    int estimateTimeRemaining(int percentage); // Returns estimated minutes remaining
    bool isLowBattery();
    
  private:
    FancyLog& fancyLog;
};

#endif // BATTERY_MONITOR_H 