#include "config.h"

#ifdef RUN_IMU

#include "bb_imu.h"

// Ground zero values
float groundZeroX = 0.0, groundZeroY = 0.0, groundZeroZ = 0.0;
bool groundZeroSet = false;

// Thresholds for feedback and rep counting
const float greenThreshold = 0.05;
const float yellowThreshold = 0.1;
const float redThreshold = 0.2;
const float squatDepthThreshold = 0.3; // Define depth for squat
const float returnToTopThreshold = 0.1; // Define return to top position

// IMU object
Adafruit_BNO08x bno = Adafruit_BNO08x();
sh2_SensorValue_t sensorValue;

// Rep counting variables
int repCount = 0;
bool isSquatting = false; // Tracks whether the user is in the squat position

// Function to calibrate the ground zero position
void calibrateGroundZero() {
  Serial.println("Calibrating ground zero. Please keep the device stationary...");
  delay(3000); // Wait for 3 seconds

  groundZeroSet = false;
  int retries = 10;
  while (retries > 0 && !groundZeroSet) {
    if (bno.getSensorEvent(&sensorValue)) {
      // Set ground zero based on the current stable position
      groundZeroX = sensorValue.un.gameRotationVector.i;
      groundZeroY = sensorValue.un.gameRotationVector.j;
      groundZeroZ = sensorValue.un.gameRotationVector.k;
      groundZeroSet = true;

      Serial.println("Ground zero calibration complete!");
      Serial.print("Ground Zero - X: "); Serial.print(groundZeroX, 6);
      Serial.print(" Y: "); Serial.print(groundZeroY, 6);
      Serial.print(" Z: "); Serial.println(groundZeroZ, 6);
    } else {
      retries--;
      delay(100);
    }
  }

  if (!groundZeroSet) {
    Serial.println("Failed to set ground zero after multiple attempts.");
  }
}

// Function to validate posture and count reps
void validatePostureAndCountReps() {
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

    // Determine the maximum deviation for feedback
    float maxDeviation = max(deltaX, max(deltaY, deltaZ));

    // Feedback for posture
    if (maxDeviation < greenThreshold) {
      Serial.println("Form is correct (Green).");
    } else if (maxDeviation < redThreshold) {
      Serial.println("Moderate deviation detected (Yellow).");
    } else {
      Serial.println("Severe deviation detected (Red).");
    }

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
      repCount++;
      Serial.print("Rep completed! Total reps: ");
      Serial.println(repCount);
    }
  } else {
    Serial.println("Failed to retrieve sensor data.");
  }

  delay(100); // Adjust update rate as needed
}

// IMU setup function
void imuSetup() {
  Serial.begin(115200);
  while (!Serial);

  if (!bno.begin_I2C()) {
    Serial.println("Failed to find BNO085 chip. Check wiring!");
    while (1); // Halt if BNO085 isn't found
  }

  Serial.println("BNO085 detected!");

  if (!bno.enableReport(SH2_GAME_ROTATION_VECTOR)) {
    Serial.println("Failed to enable Game Rotation Vector report.");
    while (1); // Halt if report isn't enabled
  }

  Serial.println("Game Rotation Vector report enabled!");

  calibrateGroundZero(); // Perform initial calibration
}

// IMU loop function
void imuLoop() {
  if (repCount < 10) {
    validatePostureAndCountReps(); // Continuously validate posture and count reps
  }
  else return;
}

#endif
