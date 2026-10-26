#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

// Motor A pins (Left Motor)
const int IN1 = 33;
const int IN2 = 25;
const int ENA = 32;

// Motor B pins (Right Motor)
const int IN3 = 26;
const int IN4 = 27;
const int ENB = 14;

// PWM settings
const int freq = 25000;  // 25kHz - above human hearing range
const int resolution = 8;

// Motor calibration
float motorA_multiplier = 1.5;   // Motor A is weaker
float motorB_multiplier = 1.0;

// PID variables - TUNE THESE!
float setpoint = 0.0;    // Target angle (0 = vertical) - adjust if robot leans one way
float Kp = 50.0;         // Proportional gain - start here
float Ki = 0.0;          // Integral gain - add later if needed
float Kd = 2.0;          // Derivative gain - reduces oscillation

float lastError = 0.0;
float integral = 0.0;

// Complementary filter variables
float pitch = 0.0;
float gyroAngleY = 0.0;
unsigned long lastTime = 0;

// Motor speed limits
const int MAX_SPEED = 255;
const int MIN_SPEED = 200;  // Minimum working speed for your motors

void setup() {
  Serial.begin(115200);
  
  // Wait for Serial to initialize - CRITICAL FOR ESP32
  while (!Serial) {
    delay(10);
  }
  
  delay(1000);
  Serial.println("\n=== Self-Balancing Robot ===\n");
  
  // Motor setup
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // Use the older ledcAttach function for compatibility
  ledcAttach(ENA, freq, resolution);
  ledcAttach(ENB, freq, resolution);
  
  Serial.println("Motors initialized...");
  
  // MPU6050 setup with I2C pins explicitly set
  Wire.begin(4, 5); // SDA, SCL
  delay(100);
  
  Serial.println("Looking for MPU6050...");
  if (!mpu.begin(0x69, &Wire)) {
    Serial.println("Failed to find MPU6050!");
    Serial.println("Check wiring:");
    Serial.println("  VCC -> 3.3V");
    Serial.println("  GND -> GND");
    Serial.println("  SCL -> GPIO 22");
    Serial.println("  SDA -> GPIO 21");
    while (1) {
      delay(1000);
      Serial.println("Still waiting for MPU6050...");
    }
  }
  Serial.println("MPU6050 Found!");
  
  // Configure MPU6050
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  Serial.println("MPU6050 configured!");
  
  Serial.println("\n*** Calibrating... Keep robot FLAT on its side! ***");
  delay(3000);
  
  // Initial sensor reading
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Calculate initial pitch from accelerometer
  pitch = atan2(a.acceleration.x, a.acceleration.z) * 180.0 / PI;
  gyroAngleY = pitch;
  
  lastTime = millis();
  
  Serial.println("\n*** READY! Stand the robot UPRIGHT now! ***");
  Serial.println("The robot will try to balance in 3 seconds...\n");
  delay(3000);
  Serial.println("GO!\n");
}

void motorA(int speed) {
  // Apply calibration multiplier
  speed = (int)(speed * motorA_multiplier);
  speed = constrain(speed, -MAX_SPEED, MAX_SPEED);
  
  // Apply minimum speed threshold
  if (abs(speed) > 0 && abs(speed) < MIN_SPEED) {
    speed = (speed > 0) ? MIN_SPEED : -MIN_SPEED;
  }
  
  if (speed > 0) {
    digitalWrite(IN1, LOW);   // REVERSED
    digitalWrite(IN2, HIGH);  // REVERSED
    ledcWrite(ENA, speed);
  } else if (speed < 0) {
    digitalWrite(IN1, HIGH);  // REVERSED
    digitalWrite(IN2, LOW);   // REVERSED
    ledcWrite(ENA, -speed);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    ledcWrite(ENA, 0);
  }
}

void motorB(int speed) {
  // Apply calibration multiplier
  speed = (int)(speed * motorB_multiplier);
  speed = constrain(speed, -MAX_SPEED, MAX_SPEED);
  
  // Apply minimum speed threshold
  if (abs(speed) > 0 && abs(speed) < MIN_SPEED) {
    speed = (speed > 0) ? MIN_SPEED : -MIN_SPEED;
  }
  
  if (speed > 0) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    ledcWrite(ENB, speed);
  } else if (speed < 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    ledcWrite(ENB, -speed);
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    ledcWrite(ENB, 0);
  }
}

void stopMotors() {
  motorA(0);
  motorB(0);
}

float calculatePID(float currentAngle) {
  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0;
  if (dt == 0) dt = 0.001; // Prevent division by zero
  lastTime = currentTime;
  
  // Calculate error
  float error = setpoint - currentAngle;
  
  // Proportional term
  float P = Kp * error;
  
  // Integral term (with anti-windup)
  integral += error * dt;
  integral = constrain(integral, -50, 50);
  float I = Ki * integral;
  
  // Derivative term
  float derivative = (error - lastError) / dt;
  float D = Kd * derivative;
  
  lastError = error;
  
  // Calculate total output
  float output = P + I + D;
  
  return output;
}

void loop() {
  // Read sensor data
  sensors_event_t a, g, temp;
  if (!mpu.getEvent(&a, &g, &temp)) {
    Serial.println("Failed to read MPU6050!");
    stopMotors();
    delay(1000);
    return;
  }
  
  // Calculate time step
  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0;
  if (dt == 0) dt = 0.001;
  
  // Calculate pitch angle from accelerometer (in degrees)
  float accelAngleY = atan2(a.acceleration.x, a.acceleration.z) * 180.0 / PI;
  
  // Complementary filter (98% gyro, 2% accel)
  pitch = 0.98 * (pitch + g.gyro.y * dt * 180.0/PI) + 0.02 * accelAngleY;
  
  // Calculate PID output
  float motorSpeed = calculatePID(pitch);
  
  // Safety check: if robot tilts too much, stop and reset
  if (abs(pitch) > 45) {
    stopMotors();
    integral = 0;
    Serial.println("*** FALLEN! Resetting... Stand upright again! ***");
    delay(1000);
  } else {
    // Apply motor speeds
    // Negative because forward motion corrects backward tilt
    motorA(-motorSpeed);
    motorB(-motorSpeed);
  }
  
  // Debug output every 100ms
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 100) {
    lastPrint = millis();
    Serial.print("Pitch: ");
    Serial.print(pitch, 1);
    Serial.print("° | Error: ");
    Serial.print(setpoint - pitch, 1);
    Serial.print("° | Motor: ");
    Serial.println((int)motorSpeed);
  }
  
  // Small delay to prevent overwhelming the ESP32
  delay(10);
}