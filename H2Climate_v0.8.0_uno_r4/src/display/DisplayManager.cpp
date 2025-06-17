#include "DisplayManager.h"

DisplayManager::DisplayManager(FancyLog& fancyLog)
    : fancyLog(fancyLog) {}

void DisplayManager::begin() {
    matrix.begin();
    showNeutralFace();
    fancyLog.toSerial("LED matrix initialized");
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
    // Create an animated sequence to indicate an update is available
    
    // First, show a special pattern with an up arrow
    uint8_t updateArrow[8][12] = {
        {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0},
        {0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0},
        {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    
    // Display arrow pattern
    matrix.renderBitmap(updateArrow, 8, 12);
    delay(1000);
    
    // Create a scrolling "U" pattern to indicate update
    uint8_t updatePattern1[8][12] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    
    // Display update letter pattern
    matrix.renderBitmap(updatePattern1, 8, 12);
    delay(800);
    
    // Alternate between arrow and happy face a few times
    for (int i = 0; i < 2; i++) {
        matrix.renderBitmap(updateArrow, 8, 12);
        delay(700);
        showHappyFace();
        delay(700);
    }
}

void DisplayManager::showUpdateProgress(int percentage) {
    // Ensure percentage is within valid range
    percentage = constrain(percentage, 0, 100);
    
    // For 100% (completed update), show celebration animation
    if (percentage == 100) {
        // Show a series of animations for completion
        for (int i = 0; i < 3; i++) {
            // Flash all pixels on
            uint8_t allOn[8][12];
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 12; x++) {
                    allOn[y][x] = 1;
                }
            }
            matrix.renderBitmap(allOn, 8, 12);
            delay(200);
            
            // Show happy face
            showHappyFace();
            delay(500);
        }
        
        // Simple checkmark for completion
        uint8_t checkmark[8][12] = {
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
            {0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
            {0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        };
        matrix.renderBitmap(checkmark, 8, 12);
        delay(2000);
        return;
    }
    
    // Create a custom pattern for the update progress display
    uint8_t progressFrame[8][12] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // Top border
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // Bottom border
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    
    // Calculate how many columns to fill based on percentage
    int columnsToFill = map(percentage, 0, 100, 0, 10);
    
    // Fill in the progress bar
    for (int row = 2; row <= 5; row++) {
        for (int col = 1; col <= columnsToFill; col++) {
            progressFrame[row][col] = 1;
        }
    }
    
    // Display the progress frame
    matrix.renderBitmap(progressFrame, 8, 12);
    
    // Every 5 seconds or on specific percentage milestones, display percentage digits
    unsigned long currentMillis = millis();
    static unsigned long lastTextMillis = 0;
    static int lastTextPercentage = -5;
    
    if (currentMillis - lastTextMillis > 5000 || 
        (percentage % 25 == 0 && percentage != lastTextPercentage)) {
        
        lastTextMillis = currentMillis;
        lastTextPercentage = percentage;
        
        // For percentage milestones, show numerical digits
        if (percentage == 25) {
            // Display "25" pattern
            uint8_t pattern25[8][12] = {
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0},
                {0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
                {0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
                {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0},
                {0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
            };
            matrix.renderBitmap(pattern25, 8, 12);
        } else if (percentage == 50) {
            // Display "50" pattern
            uint8_t pattern50[8][12] = {
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0},
                {0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0},
                {0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0},
                {0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0},
                {0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0},
                {0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
            };
            matrix.renderBitmap(pattern50, 8, 12);
        } else if (percentage == 75) {
            // Display "75" pattern
            uint8_t pattern75[8][12] = {
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0},
                {0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
                {0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
                {0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0},
                {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0},
                {0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
            };
            matrix.renderBitmap(pattern75, 8, 12);
        }
        
        delay(1500);
        
        // Return to showing the progress bar
        matrix.renderBitmap(progressFrame, 8, 12);
    }
}

void DisplayManager::showUpdateInitializing() {
    // Show an animated loading spinner to indicate update initialization
    
    // Define frames for the spinning animation
    uint8_t spinner1[8][12] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    
    uint8_t spinner2[8][12] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    
    uint8_t spinner3[8][12] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    
    // Display initial "INIT" pattern
    uint8_t initPattern[8][12] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
        {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
        {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
        {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
        {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    
    matrix.renderBitmap(initPattern, 8, 12);
    delay(800);
    
    // Display spinning animation several times
    for (int i = 0; i < 3; i++) {
        matrix.renderBitmap(spinner1, 8, 12);
        delay(300);
        matrix.renderBitmap(spinner2, 8, 12);
        delay(300);
        matrix.renderBitmap(spinner3, 8, 12);
        delay(300);
    }
}

void DisplayManager::clear() {
    matrix.clear();
} 