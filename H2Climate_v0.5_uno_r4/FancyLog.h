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
    void toSerial(String message);
    void toSerial(String message, LogLevel level);

  private:
    String getLevelString(LogLevel level);
    char getBorderChar(LogLevel level);
};

#endif
