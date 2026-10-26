#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Button pin
const int buttonPin = 14;

// Volatile variable to count presses
volatile int messageIndex = 0;

// Messages to cycle through
const char* messages[] = {
  "Hello, world!",
  "ESP32 says hi!",
  "Button was pressed!",
  "OLED is fun!",
  "Last message!"
};

const int totalMessages = sizeof(messages) / sizeof(messages[0]);

// Interrupt Service Routine (ISR)
void IRAM_ATTR handleButtonPress() {
  messageIndex++;
  if (messageIndex >= totalMessages) {
    messageIndex = 0;
  }
}

void setup() {
  Serial.begin(115200);

  // Start display
  Wire.begin(21, 22); // SDA, SCL
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  // Show first message
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(messages[messageIndex]);
  display.display();

  // Set up button pin
  pinMode(buttonPin, INPUT_PULLUP);

  // Attach interrupt
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonPress, FALLING);
}

void loop() {
  static int lastIndex = -1;

  // Only update display if message changed
  if (messageIndex != lastIndex) {
    lastIndex = messageIndex;

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(messages[messageIndex]);
    display.display();
  }

  delay(10); // Debounce buffer (software level)
}
