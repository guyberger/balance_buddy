#include "config.h"

#include "thingProperties.h"
#include "bb_imu.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO08x.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

// Select the desired sensor output
#define REPORT_TYPE SH2_GAME_ROTATION_VECTOR

// Pixels
#define BUTTON_PIN 1
#define RED_LED_PIN 2
#define BLUE_LED_PIN 3
#define VM_PIN 4
#define NEOPIXEL_PIN 7

#define NO_LIGHT 0
#define RED_MODE 1
#define YELLOW_MODE 2
#define GREEN_MODE 3
#define RED 255, 0, 0
#define YELLOW 255, 255, 0
#define GREEN 0, 255, 0
#define NO_COLOR 0, 0, 0


#define NUMPIXELS 8

int exerciseState = 0; // 0 = none; 1 = squat; 2 = row;

const float greenThreshold = 0.05;
const float yellowThreshold = 0.1;
const float redThreshold = 0.2;
int lightMode = NO_LIGHT;

Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void onRepsChange();

void setColor(int red, int green, int blue) {
 for (int i = 0; i < NUMPIXELS; i++) {
   pixels.setPixelColor(i, pixels.Color(red, green, blue));
 }
 pixels.show();
}

void updateExerciseState() {
  Serial.print("Current exerciseState is: ");
  Serial.println(exerciseState);
  exerciseState = (++exerciseState) % 3;
  Serial.print("Updated exerciseState to: ");
  Serial.println(exerciseState);

  if (exerciseState == 1) {
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, LOW);
  } else if (exerciseState == 2) {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, HIGH);
  } else {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, LOW);
  }
}


void fullSetup() {
    imuSetup();
  initProperties();
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

 pinMode(BUTTON_PIN, INPUT);
 pinMode(VM_PIN, OUTPUT);
 pinMode(RED_LED_PIN, OUTPUT);
 pinMode(BLUE_LED_PIN, OUTPUT);
 pixels.begin();
 setColor(NO_COLOR); // Turn off all LEDs initially
}


void fullLoop() {
 if (!groundZeroSet) {
   Serial.println("Ground zero not set. Recalibrating...");
   calibrateGroundZero();
   return; // Retry calibration
 }
 ArduinoCloud.update();

 int buttonState = digitalRead(BUTTON_PIN);
 if (buttonState == HIGH) {
  updateExerciseState();
 }


 if (bno.getSensorEvent(&sensorValue)) {
   float deltaX = abs(sensorValue.un.gameRotationVector.i - groundZeroX);
   float deltaY = abs(sensorValue.un.gameRotationVector.j - groundZeroY);
   float deltaZ = abs(sensorValue.un.gameRotationVector.k - groundZeroZ);


   float maxDeviation = max(deltaX, max(deltaY, deltaZ));


   if (maxDeviation < greenThreshold) {
     if (lightMode != GREEN_MODE) {
       setColor(GREEN);
       lightMode = GREEN_MODE;
       digitalWrite(VM_PIN, LOW);
     }
     Serial.println("Form is correct (Green).");
   } else if (maxDeviation < redThreshold) {
     if (lightMode != YELLOW_MODE) {
       setColor(YELLOW);
       lightMode = YELLOW_MODE;
       digitalWrite(VM_PIN, LOW);
     }
     Serial.println("Moderate deviation detected (Yellow).");
   } else {
     if (lightMode != RED_MODE) {
       setColor(RED);
       digitalWrite(VM_PIN, HIGH);
       lightMode = RED_MODE;
     }
     Serial.println("Severe deviation detected (Red).");
   }


   Serial.print("Relative X: "); Serial.print(deltaX, 6);
   Serial.print(" Y: "); Serial.print(deltaY, 6);
   Serial.print(" Z: "); Serial.println(deltaZ, 6);
 } else {
   Serial.println("Failed to retrieve sensor data.");
 }

  reps += 1;
 delay(100);
}

void onRepsChange()  {
   Serial.println("rep count: "); Serial.print(reps, 2);
}
