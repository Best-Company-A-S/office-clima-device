#include "Logger.h"

Logger::Logger() {}

void Logger::begin(unsigned long baudRate) {
    Serial.begin(baudRate);
}

void Logger::log(const String& message) {
    Serial.println(message);
}

void Logger::logWithBorder(const String& message) {
    String border = createBorder(message.length());
    Serial.println("¤" + border + "¤");
    Serial.println("|" + message + "|");
    Serial.println("¤" + border + "¤");
}

String Logger::createBorder(int length) {
    String border = "";
    for (int i = 0; i < length; i++) {
        border += "=";
    }
    return border;
} 