#ifndef CONFIG_H
#define CONFIG_H

#define RUN_FULL
// #define RUN_IMU

// Thresholds for posture validation
#define GREEN_THRESHOLD 0.05
#define YELLOW_THRESHOLD 0.1
#define RED_THRESHOLD 0.2

// Stability detection
#define REQUIRED_STABLE_READINGS 50
#define MAX_ANGLE_CHANGE 0.5  // Degrees
#define MAX_DEVIATION 10.0    // Degrees

// Calibration delay
#define CALIBRATION_DELAY 3000 // Milliseconds

// Sampling rate for IMU
#define SAMPLING_DELAY 20 // Milliseconds

#define REPORT_TYPE SH2_GAME_ROTATION_VECTOR

#endif
