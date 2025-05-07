#include "FancyLog.h"

//¤===========================¤
//| FancyLog Public Functions |
//¤===========================¤===========================================================¤
FancyLog::FancyLog() {}

void FancyLog::begin(unsigned long baudRate) {
  	Serial.begin(baudRate);

  	while(!Serial) {
    	delay(10); // Wait for Serial to be ready
  	}
}

void FancyLog::toSerial(const String& message) {
  	Serial.println("| " + message);
}

void FancyLog::toSerial(const String& message, LogLevel level) {
  	char borderChar = getBorderChar(level);

  	String levelSegment = "[" + String(getLevelString(level)) + "]";

  	String paddedMessage = " " + message + " ";

  	// Determine the border length: we want it to be at least as long as the log message
  	// and long enough to fully display the levelSegment.
  	int contentLength = paddedMessage.length();
  	int segmentLength = levelSegment.length();
  	int borderLength = contentLength;
  	if (borderLength < segmentLength) {
    	borderLength = segmentLength;
  	}

    // Build the top border with the levelSegment centered
    int remaining = borderLength - segmentLength;
    int leftCount  = remaining / 2;
    int rightCount = remaining - leftCount;

  	String borderTop = "";
  	for (int i = 0; i < leftCount; i++) {
    	borderTop += borderChar;
  	}

  	borderTop += levelSegment;
  	for (int i = 0; i < rightCount; i++) {
    	borderTop += borderChar;
  	}

  	// Build a bottom border (without the level string) with the same borderLength.
  	String borderBottom = "";
  	for (int i = 0; i < borderLength; i++) {
    	borderBottom += borderChar;
  	}

    // Output the logging result with decorative borders.
    Serial.println("¤" + borderTop     + "¤");
    Serial.println("|" + paddedMessage + "|");
    Serial.println("¤" + borderBottom  + "¤");
}

//¤============================¤
//| FancyLog Private Functions |
//¤============================¤==========================================================¤
String FancyLog::getLevelString(LogLevel level) {
  	switch (level) {
    	case INFO: return "INFO";
		case DEBUG: return "DEBUG";
    	case WARNING: return "WARNING";
    	case ERROR: return "ERROR";
    	default: return "UNKNOWN";
  	}
}

char FancyLog::getBorderChar(LogLevel level) {
  	switch (level) {
    	case INFO: return '~';
		case DEBUG: return '#';
    	case WARNING: return '=';
    	case ERROR: return '/';
    	default: return '-';
  	}
}
