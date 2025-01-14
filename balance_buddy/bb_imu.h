#ifndef BB_IMU_H
#define BB_IMU_H
#include "config.h"

#ifdef RUN_IMU

#include <Adafruit_BNO08x.h>

// Function declarations
void imuSetup();
void imuLoop();
void calibrateGroundZero();
void detectStartingPosition();
void validatePosture();
void calculatePitchRoll(float accelX, float accelY, float accelZ, float &pitch, float &roll);

#endif
#endif