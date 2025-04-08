#include "FancyLog.h"

//¤========================¤
//| Fancy Logging Function |
//¤========================¤==============================================================¤
void FancyLog::logToSerial(String message) {
  String messageBorder = "";
  String fullMessage = " " + message + " ";
  int messageLength = fullMessage.length();

  for (int i = 0; i < messageLength; i++) {
    messageBorder += "-";
  }

  Serial.println("¤" + messageBorder + "¤");
  Serial.println("|" + fullMessage   + "|");
  Serial.println("¤" + messageBorder + "¤");
}

void FancyLog::logToSerial(String message, LogLevel level) {
  char topBorderChar = getTopBorderChar(level);
  char bottomBorderChar = getBottomBorderChar(level);

  String levelString = getLevelString(level);
  String fullMessage = "[" + levelString + "] " + message + " ";
  int messageLength = fullMessage.length();

  String messageTopBorder = "";
  String messageBottomBorder = "";

  for (int i = 0; i < messageLength; i++) {
    messageTopBorder += topBorderChar;
    messageBottomBorder += bottomBorderChar;
  }

  Serial.println("¤" + messageTopBorder    + "¤");
  Serial.println("|" + fullMessage         + "|");
  Serial.println("¤" + messageBottomBorder + "¤");
}

String FancyLog::getLevelString(LogLevel level) {
  switch (level) {
    case INFO: return "INFO";
    case WARNING: return "WARNING";
    case ERROR: return "ERROR";
    default: return "UNKNOWN";
  }
}

char FancyLog::getTopBorderChar(LogLevel level) {
  switch (level) {
    case INFO: return '=';
    case WARNING: return 'v';
    case ERROR: return '\\';
    default: return '-';
  }
}

char FancyLog::getBottomBorderChar(LogLevel level) {
  switch (level) {
    case INFO: return '=';
    case WARNING: return '^';
    case ERROR: return '/';
    default: return '-';
  }
}
