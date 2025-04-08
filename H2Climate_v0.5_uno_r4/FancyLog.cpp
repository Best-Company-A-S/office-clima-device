#include "Arduino.h"
#include "TimeLib.h"  // For timestamps
#include "FancyLog.h"

FancyLog::FancyLog() {
    Serial.begin(9600);  // Ensure Serial is initialized
}

void FancyLog::log(String message, LogLevel level) {
    String timestamp = getTimestamp();
    String levelString = getLevelString(level);
    
    int messageLength = message.length();
    String border = String('=', messageLength);

    Serial.println("¤" + border + "¤");
    Serial.println("| [" + timestamp + "] [" + levelString + "] " + message + " |");
    Serial.println("¤" + border + "¤");
}

String FancyLog::getTimestamp() {
    if (timeStatus() == timeSet) {
        return String(hour()) + ":" + String(minute()) + ":" + String(second());
    }
    return "NO_TIME"; // Fallback if time isn't set
}

String FancyLog::getLevelString(LogLevel level) {
    switch (level) {
        case INFO: return "INFO";
        case WARNING: return "WARNING";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
