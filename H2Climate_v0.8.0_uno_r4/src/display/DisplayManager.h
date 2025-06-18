#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "../config/Config.h"

class DisplayManager {
  public:
    DisplayManager();
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
    ArduinoLEDMatrix matrix;
};

#endif // DISPLAY_MANAGER_H 