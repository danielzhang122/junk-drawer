
/***************************************************************
 * Lab 05: Control of a Stepper Motor using mbed on the STM32F429ZI
 *
 * Angular Resolution Calculation:
 *   - The stepper motor makes 48 full steps per revolution.
 *   - Angular resolution = 360° / 48 = 7.5° per step.
 *
 * Student Information and Time Periods Between Steps:
 *
 *   Student 1:
 *     Name   : Student A
 *     Number : 400000055
 *     T (Time per Revolution) = 55 seconds 
 *       - (Last two digits = 55, which is between 33 and 66, so T = 55)
 *       - Full-Step period: 55 / 48  ≈ 1.1458 seconds per step.
 *       - Half-Step period: 55 / 96  ≈ 0.5729 seconds per step.
 *
 *   Student 2:
 *     Name   : Student B
 *     Number : 400000073
 *     T (Time per Revolution) = 40 seconds 
 *       - (Last two digits = 73, which is > 66, so T = 73 - 33 = 40)
 *       - Full-Step period: 40 / 48  ≈ 0.8333 seconds per step.
 *       - Half-Step period: 40 / 96  ≈ 0.4167 seconds per step.
 *
 ***************************************************************/

#include "mbed.h"
#include "LCD_DISCO_F429ZI.h"
#include <cstdio>
#include "DebouncedInterrupt.h"
#include <chrono>

LCD_DISCO_F429ZI lcd;


struct Student {
    const char* name;
    uint32_t number;
    float T;  // seconds per revolution
};

// Compute T from the student number's last two digits.
float computeT(uint32_t studentNumber) {
    uint32_t lastTwo = studentNumber % 100;
    if(lastTwo < 33) {
        return lastTwo + 33;
    } else if(lastTwo <= 66) {
        return lastTwo;
    } else {
        return lastTwo - 33;
    }
}

Student student1 = {"Student A", 400000055, 0};
Student student2 = {"Student B",     400000073, 0};

Student* currentStudent = &student1;
volatile float secondsPerRev; // seconds per revolution

volatile bool isHalfStep    = false; // false: full-step, true: half-step
volatile bool isDirectionCW = true;  // true: clockwise, false: counterclockwise

// Step sequences (4-bit pattern: Bit3: AIN1, Bit2: AIN2, Bit1: BIN1, Bit0: BIN2)
// Full-step: 4-step sequence.
static const uint8_t fullStepSequence[4] = { 0xA, 0x6, 0x5, 0x9 };
// Half-step: 8-step sequence.
static const uint8_t halfStepSequence[8] = { 0x8, 0xA, 0x2, 0x6, 0x4, 0x5, 0x1, 0x9 };

volatile int fullStepIndex = 0;
volatile int halfStepIndex = 0;

DigitalOut AIN1(PA_0);
DigitalOut AIN2(PA_1);
DigitalOut BIN1(PA_2);
DigitalOut BIN2(PA_3);

Ticker stepTicker;           // Ticker to call the step function periodically
volatile float timePerStep = 0.0f;   // computed time between steps (in seconds)



InterruptIn userButton(BUTTON1);

DebouncedInterrupt modeButton(PB_0);      // Toggle between full-step and half-step.
DebouncedInterrupt dirButton(PB_1);       // Toggle motor direction.
DebouncedInterrupt speedUpButton(PB_2);     // Increase speed (reduce seconds per rev).
DebouncedInterrupt speedDownButton(PB_3);   // Decrease speed (increase seconds per rev).

char line[64];

void setMotorOutputs(uint8_t pattern) {
    // Bit 3: AIN1, Bit 2: AIN2, Bit 1: BIN1, Bit 0: BIN2.
    AIN1 = (pattern & 0x8) ? 1 : 0;
    AIN2 = (pattern & 0x4) ? 1 : 0;
    BIN1 = (pattern & 0x2) ? 1 : 0;
    BIN2 = (pattern & 0x1) ? 1 : 0;
}
  
// Called by the Ticker to advance one motor step.
void stepMotor() {
    if(isHalfStep) {
        uint8_t pattern = halfStepSequence[halfStepIndex];
        setMotorOutputs(pattern);
        if(isDirectionCW) {
            halfStepIndex = (halfStepIndex + 1) % 8;
        } else {
            halfStepIndex = (halfStepIndex - 1 + 8) % 8;
        }
    } else {
        uint8_t pattern = fullStepSequence[fullStepIndex];
        setMotorOutputs(pattern);
        if(isDirectionCW) {
            fullStepIndex = (fullStepIndex + 1) % 4;
        } else {
            fullStepIndex = (fullStepIndex - 1 + 4) % 4;
        }
    }
}

// Update the ticker period based on the current motor speed and stepping mode.
void updateTicker() {
    // Calculate time per step:
    // Full-step: secondsPerRev / 48, half-step: secondsPerRev / 96.
    if(isHalfStep) {
        timePerStep = secondsPerRev / 96.0f;
    } else {
        timePerStep = secondsPerRev / 48.0f;
    }
    // Detach and reattach the ticker with the new period.
    stepTicker.detach();
    stepTicker.attach(&stepMotor, std::chrono::duration_cast<std::chrono::microseconds>(
                       std::chrono::duration<float>(timePerStep)));
}

void updateLCD() {
    lcd.Clear(LCD_COLOR_WHITE);
    lcd.SetFont(&Font12);
    
    // Display student info on line 1.
    sprintf(line, "%s (%u)", currentStudent->name, currentStudent->number);
    lcd.DisplayStringAt(0,0, (uint8_t*)line, CENTER_MODE);
    
    // Display seconds per revolution on line 2.
    sprintf(line, "T = %.1f s/rev", secondsPerRev);
    lcd.DisplayStringAt(0, 5, (uint8_t*)line, CENTER_MODE);
    
    // Display stepping mode on line 3.
    if(isHalfStep)
        sprintf(line, "Mode: Half-Step");
    else
        sprintf(line, "Mode: Full-Step");
    lcd.DisplayStringAt(0, 10, (uint8_t*)line, CENTER_MODE);
    
    // Display motor direction on line 4.
    if(isDirectionCW)
        sprintf(line, "Dir: CW");
    else
        sprintf(line, "Dir: CCW");
    lcd.DisplayStringAt(0, 15, (uint8_t*)line, CENTER_MODE);
}

// User button ISR: Toggle between Student 1 (Student A) and Student 2 (Student B).
void userButtonISR() {
    if(currentStudent == &student1) {
        currentStudent = &student2;
    } else {
        currentStudent = &student1;
    }
    secondsPerRev = currentStudent->T;
    // Reset stepping mode to full-step and direction to CW.
    isHalfStep = false;
    isDirectionCW = true;
    fullStepIndex = 0;
    halfStepIndex = 0;
    updateTicker();
    updateLCD();
}

// Mode button ISR: Toggle full-step/half-step.
void modeButtonISR() {
    isHalfStep = !isHalfStep;
    fullStepIndex = 0;
    halfStepIndex = 0;
    updateTicker();
    updateLCD();
}

// Direction button ISR: Toggle motor rotation direction.
void dirButtonISR() {
    isDirectionCW = !isDirectionCW;
    updateLCD();
}

// Speed Up button ISR: Increase motor speed (reduce seconds per revolution).
void speedUpButtonISR() {
    if(secondsPerRev > 5.0f) { // minimum limit.
        secondsPerRev -= 1.0f;
        updateTicker();
        updateLCD();
    }
}

// Speed Down button ISR: Decrease motor speed (increase seconds per revolution).
void speedDownButtonISR() {
    if(secondsPerRev < 120.0f) { // maximum limit.
        secondsPerRev += 1.0f;
        updateTicker();
        updateLCD();
    }
}

int main() {
    // Compute T (seconds per revolution) for each student.
    student1.T = computeT(student1.number); // For Student A: last two digits = 55 -> T = 55 seconds.
    student2.T = computeT(student2.number); // For Student B: last two digits = 73 -> T = 73 - 33 = 40 seconds.
    
    // Start in Student 1 mode.
    currentStudent = &student1;
    secondsPerRev = currentStudent->T;
    
    // Initialize the LCD.
    lcd.Clear(LCD_COLOR_WHITE);
    lcd.SetFont(&Font12);
    updateLCD();
    
    // Attach button interrupts.
    // Assuming the buttons trigger on the falling edge (active low).
    userButton.fall(&userButtonISR);
    modeButton.attach(&modeButtonISR, IRQ_FALL, 10, true);
    dirButton.attach(&dirButtonISR, IRQ_FALL, 10, true);
    speedUpButton.attach(&speedUpButtonISR, IRQ_FALL, 10, true);
    speedDownButton.attach(&speedDownButtonISR, IRQ_FALL, 10, true);
    
    // Start the step ticker to drive the motor.
    updateTicker();
    
    // Main loop: All work is done in the interrupts.
    while(1) {
        ThisThread::sleep_for(100ms);
    }
}