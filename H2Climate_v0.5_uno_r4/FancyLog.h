#ifndef FancyLog_h
#define FancyLog_h

#include <Arduino.h>

enum LogLevel {
  INFO,
  WARNING,
  ERROR
};

class FancyLog {
  public:
    void logToSerial(String message);
    void logToSerial(String message, LogLevel level);

  private:
    String getLevelString(LogLevel level);
    char getTopBorderChar(LogLevel level);
    char getBottomBorderChar(LogLevel level);
};

#endif
