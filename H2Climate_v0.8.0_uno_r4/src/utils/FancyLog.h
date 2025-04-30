#ifndef FANCYLOG_H
#define FANCYLOG_H

#include <Arduino.h>

enum LogLevel {
  INFO,
  DEBUG,
  WARNING,
  ERROR
};

class FancyLog {
  public:
    void begin(unsigned long baudRate = 9600);
    void toSerial(const String& message);
    void toSerial(const String& message, LogLevel level);

  private:
    String getLevelString(LogLevel level);
    char getBorderChar(LogLevel level);
};

#endif
