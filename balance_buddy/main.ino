#include "config.h"

void fullSetup();
void fullLoop();
void imuSetup();
void imuLoop();

void setup() {
  Serial.begin(115200);
  while (!Serial);

#ifdef RUN_FULL
  fullSetup();
#endif

#ifdef RUN_IMU
  imuSetup();
#endif
}

void loop() {
#ifdef RUN_FULL
  fullLoop();
#endif

#ifdef RUN_IMU
  imuLoop();
#endif
}
