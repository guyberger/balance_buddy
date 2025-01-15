#include "bb_imu.h"

// IMU object
Adafruit_BNO08x bno = Adafruit_BNO08x();
sh2_SensorValue_t sensorValue;

// Ground zero variables
float groundZeroX = 0.0, groundZeroY = 0.0, groundZeroZ = 0.0;
bool groundZeroSet = false;

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

// Calibrate ground zero position
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
