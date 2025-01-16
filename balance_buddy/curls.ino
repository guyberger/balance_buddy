#include "squats.h"

// IMU states
const float greenThreshold = 0.05;
const float yellowThreshold = 0.1;

int evaluateDeviation(float deltaX) {   
    if (deltaX < greenThreshold) {
        return GREEN_MODE; // Form is correct
    } else if (deltaX < yellowThreshold) {
        return YELLOW_MODE; // Moderate deviation
    } else {
        return RED_MODE; // Severe deviation
    }
}

bool detectReps(float deltaY) {
    static bool goingUp = false;  // Track upward motion
    static bool goingDown = false;  // Track downward motion
    static float previousDeltaY = 0.0;

    // Check for upward motion (increasing pitch)
            Serial.print("y: ");
            Serial.print(deltaY);
        Serial.print(", prevY: ");
            Serial.println(previousDeltaY);

    if (deltaY > previousDeltaY + 0.01) { // Threshold for upward motion
        goingUp = true;
        goingDown = false;  // Reset downward flag
        Serial.println("Detected upward motion.");
    }

    // Check for downward motion (decreasing pitch)
    if (goingUp && s < previousDeltaY - 0.01) { // Threshold for downward motion
        goingDown = true;
        goingUp = false;  // Reset upward flag
        Serial.println("Detected downward motion. Rep completed!");

        previousDeltaY = deltaY;  // Update the previous pitch value
        return true;  // Rep completed
    }

    previousDeltaY = deltaY;  // Update pitch for the next iteration
    return false;  // Rep not completed
}

