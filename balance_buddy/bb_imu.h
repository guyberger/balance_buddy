#ifndef BB_IMU_H
#define BB_IMU_H

#include <Adafruit_BNO08x.h>

// IMU object and ground zero variables
extern Adafruit_BNO08x bno;
extern sh2_SensorValue_t sensorValue;

extern float groundZeroX, groundZeroY, groundZeroZ;
extern bool groundZeroSet;

// IMU function declarations
void imuSetup();
void calibrateGroundZero();

#endif
