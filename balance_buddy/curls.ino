#include "curls.h"

// IMU states
const float greenThreshold = 0.1;
const float yellowThreshold = 0.15;
float greenCtr = 0, yellowCtr = 0, redCtr = 0;
const float curlStartThresh = 0.15;

bool isCurlStarted(float deltaY) {
  bool ret = deltaY > curlStartThresh;
  return ret;
}

int evaluateDeviation(float deltaX) {   
    int ret = RED_MODE;
    if (deltaX < greenThreshold) {
        greenCtr++;
        ret = GREEN_MODE; // Form is correct
    } else if (deltaX < yellowThreshold) {
        yellowCtr++;
        ret = YELLOW_MODE; // Moderate deviation
    } else {
        redCtr++;
    }
    float sum = (yellowCtr + greenCtr + redCtr);
    if (sum == 0) {
      Serial.println("Division by 0");
      return ret;
    }
    pctGreen = (greenCtr / sum) * 100;
    return ret;
}

bool detectReps(float deltaY) {
    static bool goingUp = false;  // Track upward motion
    static bool goingDown = false;  // Track downward motion
    static float previousDeltaY = 0.0;

    // Check for upward motion (increasing pitch)
        //     Serial.print("y: ");
        //     Serial.print(deltaY);
        // Serial.print(", prevY: ");
        //     Serial.println(previousDeltaY);

    if (deltaY > previousDeltaY + 0.02) { // Threshold for upward motion
        goingUp = true;
        goingDown = false;  // Reset downward flag
        Serial.println("Detected upward motion.");
    }

    // Check for downward motion (decreasing pitch)
    if (goingUp && deltaY < previousDeltaY - 0.02) { // Threshold for downward motion
        goingDown = true;
        goingUp = false;  // Reset upward flag
        Serial.println("Detected downward motion. Rep completed!");
        previousDeltaY = deltaY;  // Update the previous pitch value
        return true;  // Rep completed
    }

    previousDeltaY = deltaY;  // Update pitch for the next iteration
    return false;  // Rep not completed
}

