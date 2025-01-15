#include "config.h"

void fullSetup();
void fullLoop();

void setup() {
  Serial.begin(115200);
  while (!Serial);

  fullSetup();
}

void loop() {
  fullLoop();
}
