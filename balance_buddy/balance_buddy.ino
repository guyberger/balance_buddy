#include "thingProperties.h"
#include "squats.h"
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BNO08x.h>

#define LED_PIN    7
#define LED_COUNT  8
#define NO_LIGHT 0
#define RED_MODE 1
#define YELLOW_MODE 2
#define GREEN_MODE 3

// Pixels
#define BUTTON_PIN 1
#define RED_LED_PIN 2
#define BLUE_LED_PIN 3
#define VM_PIN 4
#define NEOPIXEL_PIN 7
#define NUMPIXELS 8

// Colors
#define RED 255, 0, 0
#define YELLOW 255, 255, 0
#define GREEN 0, 255, 0
#define NO_COLOR 0, 0, 0
#define READY 125, 125, 125

// Leds
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

/* IMU */ 
Adafruit_BNO08x bno = Adafruit_BNO08x();
sh2_SensorValue_t sensorValue;

// Ground zero variables
float groundZeroX = 0.0, groundZeroY = 0.0, groundZeroZ = 0.0;
bool groundZeroSet = false;
bool startExercise = false;

void initProperties () {
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(reps, READWRITE, ON_CHANGE, initCount);
  ArduinoCloud.addProperty(lightMode, READWRITE, ON_CHANGE, initState);
  ArduinoCloud.addProperty(pctGreen, READWRITE, ON_CHANGE, initPct);
  ArduinoCloud.addProperty(armVertical, READWRITE, ON_CHANGE, initArm);
}

// IMU setup function
void imuSetup() {
  Serial.begin(115200);
  startExercise = false;

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

void setup() {
  Serial.begin(9600);
  
  // IMU setup
  imuSetup();
  
  // Cloud Setup
  initProperties();
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  connectToWifi();

  pixels.begin();
  setColor(READY);
}

void loop() {
    ArduinoCloud.update();

    if (bno.getSensorEvent(&sensorValue)) {
        // Evaluate squat form and deviation

           float deltaX = abs(sensorValue.un.gameRotationVector.i - groundZeroX);
          float deltaY = abs(sensorValue.un.gameRotationVector.j - groundZeroY);
          float deltaZ = abs(sensorValue.un.gameRotationVector.k - groundZeroZ);
  
        if (!startExercise) {
          if (!isCurlStarted(deltaY)) {
            delay(20);
            return;
          } else {
            startExercise = true;
            Serial.println("Started the exercise!");
          }
        }
        int deviationMode = evaluateDeviation(deltaX);
        float maxDeviation = max(deltaX, max(deltaY, deltaZ));
        // Serial.print("dx: ");
        //     Serial.print(deltaX);
        // Serial.print(", dy: ");
        //     Serial.print(deltaY);
        // Serial.print(", dz: ");
        //     Serial.print(deltaZ);
        // Serial.print(", max: ");
        //     Serial.println(maxDeviation);

        // Update light mode and provide feedback based on deviation mode
        SetMode(deviationMode);

        // Detect reps
        if (detectReps(deltaY)) {
            reps++;
            Serial.print("Curls completed: ");
            Serial.println(reps);
        }
        armVertical = deltaY;
    } else {
        Serial.println("Failed to retrieve sensor data.");
    }
    delay(20);
}


void SetMode(int mode) {
  if (lightMode != mode) {
    switch (mode) {
      case RED_MODE:
        setColor(RED);
        digitalWrite(VM_PIN, HIGH);
        Serial.println("Severe deviation detected (Red).");
      break;
      case YELLOW_MODE:
        setColor(YELLOW);
        digitalWrite(VM_PIN, LOW);
        Serial.println("Moderate deviation detected (Yellow).");
      break;
      case GREEN_MODE:
        setColor(GREEN);
        digitalWrite(VM_PIN, LOW);
        Serial.println("Form is correct (Green).");
      break;
      default:
        setColor(NO_COLOR);
      break;
    }
    lightMode = mode;
    ArduinoCloud.update();
  }
}

void setColor(int red, int green, int blue) {
 for (int i = 0; i < NUMPIXELS; i++) {
   pixels.setPixelColor(i, pixels.Color(red, green, blue));
 }
 pixels.show();
}

void connectToWifi() {
  if (WiFi.begin(SSID, PASS) == WL_CONNECTED) {
    Serial.println("Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Failed to connect to Wi-Fi.");
  }
}

void initCount() {
  reps = 0;
  ArduinoCloud.update();
}

void initPct() {
  pctGreen = 0;
  ArduinoCloud.update();
}

void initArm() {
  armVertical = 0;
  ArduinoCloud.update();
}

void initState() {
  lightMode = NO_LIGHT;
  ArduinoCloud.update();
}

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
    setColor(GREEN);
  } else {
    retries--;
    delay(100);
  }
}

if (!groundZeroSet) {
  Serial.println("Failed to set ground zero after multiple attempts.");
}
}


