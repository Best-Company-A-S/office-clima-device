#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

// Include ArduinoGraphics before Arduino_LED_Matrix
#include "../config/Config.h"

// Define scrolling direction constants if not defined
#ifndef SCROLL_LEFT
#define SCROLL_LEFT 0
#endif

#ifndef SCROLL_RIGHT 
#define SCROLL_RIGHT 1
#endif

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