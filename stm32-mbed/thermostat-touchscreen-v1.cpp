#include "mbed.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"

// Create a thread for touchscreen reading
Thread touchThread;
const int minXminus = 50;
const int maxXminus = 120;
const int minYminus = 220;
const int maxYminus = 280;

const int minXplus = 380;
const int maxXplus = 450;
const int minYplus = 220;
const int maxYplus = 280;

int startingTemp = 25;
bool pressed = false; //debouncer

AnalogIn analog_input(PA_0);
float sensorTemperature;


// Function to draw the user interface
void display(float temp) {
    BSP_LCD_Clear(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font12);
    
    // Display the current threshold temperature at the top
    char tempBuffer[50]; //not fully sure what this does tbh
    sprintf(tempBuffer, "preferred temperature %d", startingTemp);
    BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)tempBuffer, CENTER_MODE);

    
    // - button
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillRect(minXminus, minYminus, //red rectangle
                     maxXminus - minXminus, 
                     maxYminus - minYminus);

    
    // + button
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_FillRect(minXplus, minYplus, //green rectangle
                     maxXplus - minXplus, 
                     maxYplus - minYplus);


    sprintf(tempBuffer, "Sensor: %.2f C", temp);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(0, 80, (uint8_t*)tempBuffer, CENTER_MODE);

}

void tempSensor(){
    float volts, temperature, tempC;
    uint16_t volts_int;

    analog_input.set_reference_voltage(3);
  
    while (true) {
        volts = analog_input.read_voltage();
        temperature = volts/0.01; //farenheit

        tempC = (temperature-30)/2; //Celcius

        sensorTemperature = tempC;
        printf("temperature : %.2f \n ",tempC);
        ThisThread::sleep_for(500ms);
        display(sensorTemperature); //display temp

    }
}

void touchTask() {
    TS_StateTypeDef tsState;
    
    while (true) {
        
        BSP_TS_GetState(&tsState); // Read touchscreen state
        char buffer[50];
        if (tsState.TouchDetected) {
            if (pressed == false){ // to avoid holding it down
                
                int x = tsState.X;  // X coordinate
                int y = tsState.Y;  // Y coordinate
                printf("%d , %d\n",x,y);


                // Check if the touch is inside the "-" button area
                if (x >= minXminus && x <= maxXminus &&
                    y >= minYminus-200 && y <= maxYminus-200) {
                    display(sensorTemperature); //update display
                    startingTemp--;
                    printf("%d\n",startingTemp);
                    pressed = true;
                    
                }
                // Check if the touch is inside the "+" button area
                else if (x >= minXplus -230 && x <= maxXplus -230 &&
                            y >= minYplus-200 && y <= maxYplus-200) {
                    display(sensorTemperature); 
                    startingTemp++;
                    printf("%d\n",startingTemp);
                    pressed = true;
                }
                
                
            }
        } else pressed =false;

        ThisThread::sleep_for(100ms); // Small delay to avoid excessive refresh
    }
}


int main() {

    // Initialize LCD
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(0, LCD_FRAME_BUFFER);
    BSP_LCD_SelectLayer(0);
    BSP_LCD_Clear(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font24);


    display(sensorTemperature); 
    // Initialize Touchscreen
    BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());

    // Start touch input thread
    touchThread.start(touchTask);

    tempSensor();
    while (true) {
        ThisThread::sleep_for(500ms);
    }
}