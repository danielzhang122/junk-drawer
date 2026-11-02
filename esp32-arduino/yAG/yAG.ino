#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Servo myServo;
const int servoPin = 13;
const int buttonCW = 12;   // Clockwise button
const int buttonCCW = 14;  // Counter-clockwise button

// Button state variables
volatile bool buttonCWState = false;
volatile bool buttonCCWState = false;
volatile unsigned long lastCWInterruptTime = 0;
volatile unsigned long lastCCWInterruptTime = 0;
const unsigned long debounceTime = 50; // Debounce time in milliseconds

// Servo position variables
int pos = 90;
const int minAngle = 0;
const int maxAngle = 180;
const int stepSize = 10;
unsigned long lastMoveTime = 0;
const int moveInterval = 40; // ms between each move while holding

// === Interrupt Service Routines ===
void IRAM_ATTR handleButtonCW() {
  unsigned long interruptTime = millis();
  // Debounce
  if (interruptTime - lastCWInterruptTime > debounceTime) {
    buttonCWState = !digitalRead(buttonCW); // Read the actual state
    lastCWInterruptTime = interruptTime;
  }
}

void IRAM_ATTR handleButtonCCW() {
  unsigned long interruptTime = millis();
  // Debounce
  if (interruptTime - lastCCWInterruptTime > debounceTime) {
    buttonCCWState = !digitalRead(buttonCCW); // Read the actual state
    lastCCWInterruptTime = interruptTime;
  }
}

void setup() {
  Serial.begin(115200);
  
  // OLED Setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 not found");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  
  // Button Setup (INPUT_PULLUP means buttons are active LOW)
  pinMode(buttonCW, INPUT_PULLUP);
  pinMode(buttonCCW, INPUT_PULLUP);
  
  // Attach a single interrupt to each button that responds to any change
  attachInterrupt(digitalPinToInterrupt(buttonCW), handleButtonCW, CHANGE);
  attachInterrupt(digitalPinToInterrupt(buttonCCW), handleButtonCCW, CHANGE);
  
  // Servo Setup
  ESP32PWM::allocateTimer(0);
  myServo.setPeriodHertz(50);
  myServo.attach(servoPin, 500, 2400);
  myServo.write(pos);
  
  updateDisplay();
  Serial.println("System initialized");
}

void loop() {
  unsigned long now = millis();
  
  // Check for button presses directly in the main loop
  // This is more reliable than relying solely on interrupt state variables
  bool currentCWState = !digitalRead(buttonCW);    // Active LOW
  bool currentCCWState = !digitalRead(buttonCCW);  // Active LOW
  
  // Update state with both the interrupt flag and current reading
  bool cwPressed = buttonCWState || currentCWState;
  bool ccwPressed = buttonCCWState || currentCCWState;
  
  // Move if enough time has passed since last move
  if (now - lastMoveTime >= moveInterval) {
    
    if (cwPressed && pos < maxAngle) {
      pos += stepSize;
      if (pos > maxAngle) pos = maxAngle;
      myServo.write(pos);
      updateDisplay();
      lastMoveTime = now;
      Serial.print("CW: ");
      Serial.println(pos);
    } 
    else if (ccwPressed && pos > minAngle) {
      pos -= stepSize;
      if (pos < minAngle) pos = minAngle;
      myServo.write(pos);
      updateDisplay();
      lastMoveTime = now;
      Serial.print("CCW: ");
      Serial.println(pos);
    }
  }
  
  // Short delay to prevent CPU hogging
  delay(5);
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("Angle: ");
  display.print(pos);
  
  // Add indicator for button press
  display.setCursor(0, 40);
  if (buttonCWState) display.print("CW");
  if (buttonCCWState) display.print("CCW");
  
  display.display();
}