#include "DisplayManager.h"

DisplayManager::DisplayManager() {}

void DisplayManager::begin() {
    matrix.begin();
    showNeutralFace();
}

void DisplayManager::showHappyFace() {
    matrix.loadFrame(LEDMATRIX_EMOJI_HAPPY);
}

void DisplayManager::showSadFace() {
    matrix.loadFrame(LEDMATRIX_EMOJI_SAD);
}

void DisplayManager::showNeutralFace() {
    matrix.loadFrame(LEDMATRIX_EMOJI_BASIC);
}

void DisplayManager::showRetryAnimation() {
    for (int i = 0; i < RETRY_ANIMATION_BLINKS; i++) {
        showNeutralFace();
        delay(RETRY_ANIMATION_ON_TIME);
        clear();
        delay(RETRY_ANIMATION_OFF_TIME);
    }
}

void DisplayManager::showUpdateAvailable() {
    for (int i = 0; i < 3; i++) {
        showHappyFace();
        delay(500);
        showNeutralFace();
        delay(500);
    }
}

void DisplayManager::clear() {
    matrix.clear();
} 