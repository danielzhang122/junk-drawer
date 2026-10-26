#include <BluetoothSerial.h>

BluetoothSerial SerialBT;
const int ledPin = 2; // LED on GPIO2

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  SerialBT.begin("ESP32_Server"); // Server name
  Serial.println("Device ready. Pair via Bluetooth!");
}

void loop() {
  if (SerialBT.connected()) {
    if (SerialBT.available()) {
      char command = SerialBT.read();
      digitalWrite(ledPin, command == '1' ? HIGH : LOW);
    }
  }
  delay(20);
}