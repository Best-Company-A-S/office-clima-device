#ifndef FancyLog_h
#define FancyLog_h

#include "Arduino.h"
#include "TimeLib.h"  // For timestamps

class FancyLog {
public:
    enum LogLevel {
        INFO,
        WARNING,
        ERROR
    };

    FancyLog();  
    void log(String message, LogLevel level = INFO);

private:
    String getTimestamp();
    String getLevelString(LogLevel level);
};

#endif
