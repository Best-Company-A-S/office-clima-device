#ifndef LOGGER_H
#define LOGGER_H

#include "../config/Config.h"

class Logger {
public:
    Logger();
    void begin(unsigned long baudRate = 9600);
    void log(const String& message);
    void logWithBorder(const String& message);

private:
    String createBorder(int length);
};

#endif // LOGGER_H 