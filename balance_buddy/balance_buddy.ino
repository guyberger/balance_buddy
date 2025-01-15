#include "thingProperties.h"
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BNO08x.h>

#define LED_PIN    7
#define LED_COUNT  8
#define NO_LIGHT 0
#define RED_MODE 1
#define YELLOW_MODE 2
#define GREEN_MODE 3
#define STATE_GREEN "Green"
#define STATE_YELLOW "Yellow"
#define STATE_RED "Red"
#define STATE_INIT "Initializing"

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

// Leds
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

/* IMU */ 
Adafruit_BNO08x bno = Adafruit_BNO08x();
sh2_SensorValue_t sensorValue;
// IMU states
const float greenThreshold = 0.05;
const float yellowThreshold = 0.1;
const float redThreshold = 0.2;
int lightMode = NO_LIGHT;
// Ground zero variables
float groundZeroX = 0.0, groundZeroY = 0.0, groundZeroZ = 0.0;
bool groundZeroSet = false;

void initProperties () {
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(reps, READWRITE, ON_CHANGE, initCount);
  ArduinoCloud.addProperty(state, READWRITE, ON_CHANGE, initState);
}

// IMU setup function
void imuSetup() {
Serial.begin(115200);
// while (!Serial);

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
  // IMU setup
  imuSetup();
  
  // Cloud Setup
  initProperties();
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  // Connect to Arduino IoT Cloud
  connectToWifi();

  // put your setup code here, to run once:
  pixels.begin();
  pixels.show(); // Initialize all pixels to 'off'
}

void loop() {
  Serial.println("reps: "); Serial.println(reps);
  ArduinoCloud.update();

  // put your main code here, to run repeatedly:
  pixels.fill(7, 0, pixels.Color(255, 0, 255));
  pixels.show();

  if (bno.getSensorEvent(&sensorValue)) {
   float deltaX = abs(sensorValue.un.gameRotationVector.i - groundZeroX);
   float deltaY = abs(sensorValue.un.gameRotationVector.j - groundZeroY);
   float deltaZ = abs(sensorValue.un.gameRotationVector.k - groundZeroZ);


   float maxDeviation = max(deltaX, max(deltaY, deltaZ));


   if (maxDeviation < greenThreshold) {
     if (lightMode != GREEN_MODE) {
      SetMode(GREEN_MODE);
      //  setColor(GREEN);
      //  lightMode = GREEN_MODE;
      //  digitalWrite(VM_PIN, LOW);
     }
     Serial.println("Form is correct (Green).");
   } else if (maxDeviation < redThreshold) {
     if (lightMode != YELLOW_MODE) {
      SetMode(YELLOW_MODE);
      //  setColor(YELLOW);
      //  lightMode = YELLOW_MODE;
      //  digitalWrite(VM_PIN, LOW);
     }
     Serial.println("Moderate deviation detected (Yellow).");
   } else {
     if (lightMode != RED_MODE) {
      SetMode(RED_MODE);
      //  setColor(RED);
      //  digitalWrite(VM_PIN, HIGH);
      //  lightMode = RED_MODE;
     }
     Serial.println("Severe deviation detected (Red).");
   }


  //  Serial.print("Relative X: "); Serial.print(deltaX, 6);
  //  Serial.print(" Y: "); Serial.print(deltaY, 6);
  //  Serial.print(" Z: "); Serial.println(deltaZ, 6);
 } else {
   Serial.println("Failed to retrieve sensor data.");
 }

  reps++;
  delay(500);
}

void SetMode(int mode) {
  switch (mode) {
    case RED_MODE:
      setColor(RED);
      digitalWrite(VM_PIN, HIGH);
      state = STATE_RED;
    break;
    case YELLOW_MODE:
      setColor(YELLOW);
      digitalWrite(VM_PIN, LOW);
      state = STATE_YELLOW;
    break;
    case GREEN_MODE:
      setColor(GREEN);
      digitalWrite(VM_PIN, LOW);
      state = STATE_GREEN;
    break;
    default:
      setColor(NO_COLOR);
      state = STATE_INIT;
    break;
  }
  ArduinoCloud.update();
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

void initState() {
  state = STATE_INIT;
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
  } else {
    retries--;
    delay(100);
  }
}

if (!groundZeroSet) {
  Serial.println("Failed to set ground zero after multiple attempts.");
}
}

