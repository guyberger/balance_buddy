#include "squats.h"

// Variables specific to squats
int squatRepCount = 0;
bool isSquatting = false; // Tracks whether the user is in the squat position

// Thresholds for squats
const float squatDepthThreshold = 0.3;      // Define depth for squat
const float returnToTopThreshold = 0.1;    // Define return to top position

// Initialize squats
void initializeSquats() {
  Serial.println("Initializing squats...");
  calibrateGroundZero(); // Use IMU's ground zero calibration
  squatRepCount = 0;
  isSquatting = false;
  Serial.println("Squats initialized.");
}

// Execute squats logic (called in a loop)
void executeSquats() {
  if (!groundZeroSet) {
    Serial.println("Ground zero not set. Recalibrating...");
    calibrateGroundZero();
    return;
  }

  // Retrieve sensor data
  if (bno.getSensorEvent(&sensorValue)) {
    // Calculate deviations from the ground zero
    float deltaX = abs(sensorValue.un.gameRotationVector.i - groundZeroX);
    float deltaY = abs(sensorValue.un.gameRotationVector.j - groundZeroY);
    float deltaZ = abs(sensorValue.un.gameRotationVector.k - groundZeroZ);

    // Debugging: Print raw delta values
    Serial.print("Relative X: "); Serial.print(deltaX, 6);
    Serial.print(" Y: "); Serial.print(deltaY, 6);
    Serial.print(" Z: "); Serial.println(deltaZ, 6);

    // Rep counting logic
    if (deltaZ > squatDepthThreshold && !isSquatting) {
      // User is in the squat position
      isSquatting = true;
      Serial.println("Downward motion detected.");
    } else if (deltaZ < returnToTopThreshold && isSquatting) {
      // User returned to the top position
      isSquatting = false;
      squatRepCount++;
      Serial.print("Rep completed! Total reps: ");
      Serial.println(squatRepCount);
    }
  } else {
    Serial.println("Failed to retrieve sensor data.");
  }

  delay(100); // Adjust update rate as needed
}
