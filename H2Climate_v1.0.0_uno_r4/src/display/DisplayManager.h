#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "../config/Config.h"
#include "../utils/FancyLog.h"

class DisplayManager {
  public:
    DisplayManager(FancyLog& fancyLog);
    void begin();
    void showHappyFace();
    void showSadFace();
    void showNeutralFace();
    void showRetryAnimation();
    void showUpdateAvailable();
    void showUpdateProgress(int percentage);
    void showUpdateInitializing();
    void clear();

  private:
    FancyLog& fancyLog;
    ArduinoLEDMatrix matrix;
};

#endif // DISPLAY_MANAGER_H 