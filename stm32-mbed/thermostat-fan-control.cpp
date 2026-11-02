#include "mbed.h"
#include "LCD_DISCO_F429ZI.h"
#include "TS_DISCO_F429ZI.h"

LCD_DISCO_F429ZI LCD;
TS_DISCO_F429ZI TS;

// Touchscreen coordinates
const int minXminus = 40;
const int maxXminus = 100;
const int minYminus = 220;
const int maxYminus = 280;
const int minXplus = 380;
const int maxXplus = 450;
const int minYplus = 220;
const int maxYplus = 280;


float thresholdTemp = 28.0;
bool pressed = false;
bool maxSpeed = false;

AnalogIn analog_input(PA_0);
AnalogOut analog_out(PA_5);
float sensorTemperature = 0.0;

Ticker tempTicker;
Ticker touchTicker;

void display(float temp) {
    LCD.Clear(LCD_COLOR_WHITE);
    LCD.SetTextColor(LCD_COLOR_BLACK);
    LCD.SetFont(&Font12);
    char tempBuffer[50];
    sprintf(tempBuffer, "Threshold temperature %.2f C", thresholdTemp);
    LCD.DisplayStringAt(0, 50, (uint8_t*)tempBuffer, CENTER_MODE);
    
    LCD.SetTextColor(LCD_COLOR_BLUE);
    LCD.FillRect(minXminus, minYminus, maxXminus - minXminus, maxYminus - minYminus);
    LCD.SetTextColor(LCD_COLOR_RED);
    LCD.FillRect(minXplus, minYplus, maxXplus - minXplus, maxYplus - minYplus);
    
    sprintf(tempBuffer, "Sensor temperature: %.2f C", temp);
    LCD.SetTextColor(LCD_COLOR_BLACK);
    LCD.DisplayStringAt(0, 80, (uint8_t*)tempBuffer, CENTER_MODE);
}

void readTempSensor() {
    float volts = analog_input.read();  // Read normalized value (0.0 - 1.0)
    volts *= 3.0;  // Scale to reference voltage

    if (volts > 0.01) {
        float temperature = volts / 0.01; // Fahrenheit
        sensorTemperature = (temperature - 32) / 1.8; // Convert to Celsius
    } else {
        sensorTemperature = 0.0;
    }

    display(sensorTemperature);

    // Control fan based on temperature threshold
    if (sensorTemperature > thresholdTemp && !maxSpeed) {
        for (float i = 0.0f; i <= 1.0f; i += 0.05f) {  // Ramp up
            analog_out.write(i);
            ThisThread::sleep_for(100ms);
        }
        maxSpeed = true;
        analog_out.write(1.0f);
    }
    else if (sensorTemperature <= thresholdTemp) {
        analog_out.write(0.0f);
        maxSpeed = false;
    }
}

void checkTouch() {
    TS_StateTypeDef tsState;
    TS.GetState(&tsState);

    if (tsState.TouchDetected) {
        if (!pressed) {
            int x = tsState.X;
            int y = tsState.Y;

            if (x >= 30 && x <= 100 &&
                y >= 30 && y <= 80) {
                thresholdTemp--;
                LCD.SetTextColor(LCD_COLOR_GREEN);
                LCD.FillRect(minXminus, minYminus, maxXminus - minXminus, maxYminus - minYminus);
                display(sensorTemperature);
            } 
            else if (x >= 130 && x <= 200 &&
                     y >= 30 && y <= 80) {
                thresholdTemp++;
                LCD.SetTextColor(LCD_COLOR_GREEN);
                LCD.FillRect(minXplus, minYplus, maxXplus - minXplus, maxYplus - minYplus);
                display(sensorTemperature);
            }

            pressed = true;
        }
    } else {
        pressed = false;
    }
}

int main() {
    analog_input.set_reference_voltage(3.0);

    LCD.Init();
    LCD.LayerDefaultInit(0, LCD_FRAME_BUFFER);
    LCD.SelectLayer(0);
    LCD.Clear(LCD_COLOR_WHITE);
    LCD.SetTextColor(LCD_COLOR_BLACK);
    LCD.SetFont(&Font24);

    readTempSensor();
    thresholdTemp = analog_input.read() / 0.01 + 1.0;

    display(sensorTemperature);
    TS.Init(LCD.GetXSize(), LCD.GetYSize());

    touchTicker.attach(&checkTouch, 200ms);
    
    while (true) {
        readTempSensor();
        thread_sleep_for(1000);
    }
}