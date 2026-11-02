#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Servo objects
Servo rightServo;
Servo leftServo;

// Pin setup
const int rightServoPin = 13;
const int leftServoPin = 27;    // pick a PWM-capable GPIO (27 is good)
const int rightButtonPin = 12;
const int leftButtonPin = 14;

// Punch state flags
volatile bool punchRightFlag = false;
volatile bool punchLeftFlag = false;

// Servo angles
const int rightRestAngle = 180;
const int rightPunchAngle = 90;
const int leftRestAngle = 0;
const int leftPunchAngle = 90;

const int punchHoldTime = 150; // how long to hold punch (ms)
const int returnDelay = 150;   // pause before allowing next punch

// === Interrupt Handlers ===
void IRAM_ATTR handleRightPunch() { punchRightFlag = true; }
void IRAM_ATTR handleLeftPunch() { punchLeftFlag = true; }

void setup() {
  Serial.begin(115200);

  // === Setup Buttons ===
  pinMode(rightButtonPin, INPUT_PULLUP);
  pinMode(leftButtonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(rightButtonPin), handleRightPunch, FALLING);
  attachInterrupt(digitalPinToInterrupt(leftButtonPin), handleLeftPunch, FALLING);

  // === Setup Servos ===
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  rightServo.setPeriodHertz(50);
  leftServo.setPeriodHertz(50);

  rightServo.attach(rightServoPin, 500, 2400);
  leftServo.attach(leftServoPin, 500, 2400);
  rightServo.write(rightRestAngle);
  leftServo.write(leftRestAngle);

  // === Setup OLED ===
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  updateDisplay(rightRestAngle, leftRestAngle);
}

void loop() {
  if (punchRightFlag) {
    punchRightFlag = false;
    rightServo.write(rightPunchAngle);
    updateDisplay(rightPunchAngle, leftServo.read());
    delay(punchHoldTime);
    rightServo.write(rightRestAngle);
    updateDisplay(rightRestAngle, leftServo.read());
    delay(returnDelay);
  }

  if (punchLeftFlag) {
    punchLeftFlag = false;
    leftServo.write(leftPunchAngle);
    updateDisplay(rightServo.read(), leftPunchAngle);
    delay(punchHoldTime);
    leftServo.write(leftRestAngle);
    updateDisplay(rightServo.read(), leftRestAngle);
    delay(returnDelay);
  }
}

void updateDisplay(int rightAngle, int leftAngle) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("R: ");
  display.print(rightAngle);
  display.setCursor(0, 30);
  display.print("L: ");
  display.print(leftAngle);
  display.display();
}
