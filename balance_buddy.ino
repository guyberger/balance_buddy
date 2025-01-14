#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO08x.h>
#include <Adafruit_NeoPixel.h>


// Select the desired sensor output
#define REPORT_TYPE SH2_GAME_ROTATION_VECTOR


// Pixels
#define PIN 7
#define VM_PIN 2
#define NO_LIGHT 0
#define RED_MODE 1
#define YELLOW_MODE 2
#define GREEN_MODE 3
#define RED 255, 0, 0
#define YELLOW 255, 255, 0
#define GREEN 0, 255, 0
#define NO_COLOR 0, 0, 0


#define NUMPIXELS 8


float groundZeroX = 0.0, groundZeroY = 0.0, groundZeroZ = 0.0;
bool groundZeroSet = false;


const float greenThreshold = 0.05;
const float yellowThreshold = 0.1;
const float redThreshold = 0.2;
int lightMode = NO_LIGHT;


Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_BNO08x bno = Adafruit_BNO08x();
sh2_SensorValue_t sensorValue;


void setColor(int red, int green, int blue) {
 for (int i = 0; i < NUMPIXELS; i++) {
   pixels.setPixelColor(i, pixels.Color(red, green, blue));
 }
 pixels.show();
}


void calibrateGroundZero() {
 Serial.println("Calibrating ground zero. Please keep the device stationary...");
 delay(3000); // Wait for 3 seconds


 groundZeroSet = false;
 int retries = 10;
 while (retries > 0 && !groundZeroSet) {
   if (bno.getSensorEvent(&sensorValue)) {
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
   setColor(RED); // Signal error
 }
}


void setup() {
 Serial.begin(115200);
 while (!Serial);

  pinMode(VM_PIN, OUTPUT);


 if (!bno.begin_I2C()) {
   Serial.println("Failed to find BNO085 chip. Check wiring!");
   setColor(RED); // Signal error
   while (1); // Halt if BNO085 isn't found
 }


 Serial.println("BNO085 detected!");


 if (!bno.enableReport(REPORT_TYPE)) {
   Serial.println("Failed to enable Game Rotation Vector report.");
   setColor(RED); // Signal error
   while (1); // Halt if report isn't enabled
 }


 Serial.println("Game Rotation Vector report enabled!");
 pixels.begin();
 setColor(NO_COLOR); // Turn off all LEDs initially


 calibrateGroundZero();
}


void loop() {
 if (!groundZeroSet) {
   Serial.println("Ground zero not set. Recalibrating...");
   calibrateGroundZero();
   return; // Retry calibration
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


 delay(100);
}