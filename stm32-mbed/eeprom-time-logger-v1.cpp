#include "LCD_DISCO_F429ZI.h"
#include "fonts.h"
#include "mbed.h"
#include <time.h>
#include <cstdio>
#include <cstring>
#include "DebouncedInterrupt.h"

#define SDA_PIN PC_9
#define SCL_PIN PA_8
#define EEPROM_ADDR 0xA0

LCD_DISCO_F429ZI LCD;
I2C i2c(SDA_PIN, SCL_PIN);

// Buttons: 
//   User button logs time, 
//   Ext button toggles log view, 
//   Set button enters/cycles/exits set-time mode, 
//   Inc button increments the selected field.
InterruptIn UserBtn(BUTTON1);  
DebouncedInterrupt ExtBtn(PA_5);   
DebouncedInterrupt SetBtn(PA_6);   
DebouncedInterrupt IncBtn(PA_7);

// System states
enum State { IDLE_MODE, VIEW_LOGS, SET_TIME };
State system_state = IDLE_MODE;

// Global variables for set-time mode
tm newTime;
int selectedField = 0;  // 0 = Hour, 1 = Minute, 2 = Second, 3 = Day, 4 = Month, 5 = Year
bool inSetTimeMode = false;

// Flags set by ISRs
volatile bool logTimeRequested = false;
volatile bool viewLogsRequested = false;
volatile bool setTimeModeRequested = false;
volatile bool incTimeRequested = false;

// Function prototypes
void WriteEEPROM(int address, unsigned int eeaddress, char *data, int size);
void ReadEEPROM(int address, unsigned int eeaddress, char *data, int size);
void logTimeToEEPROM();
void viewLogs();
void setTimeMode();
void incrementTime();
void displayCurrentTime();

// --- ISRs: Only set flags ---
void logTimeISR() {
    logTimeRequested = true;
}

void viewLogsISR() {
    viewLogsRequested = true;
}

void setTimeModeISR() {
    setTimeModeRequested = true;
}

void incTimeISR() {
    incTimeRequested = true;
}

int main() {
    // Use a smaller font so all text fits on the LCD.
    LCD.SetFont(&Font12);
    LCD.SetTextColor(LCD_COLOR_BLACK);

    // Initialize RTC: January 1, 2025, 00:00:00.
    tm t;
    t.tm_year = 125;  // 2025 - 1900
    t.tm_mon  = 0;
    t.tm_mday = 1;
    t.tm_hour = 0;
    t.tm_min  = 0;
    t.tm_sec  = 0;
    set_time(mktime(&t));

    // Attach interrupts.
    UserBtn.fall(&logTimeISR);
    ExtBtn.attach(&viewLogsISR, IRQ_FALL, 10, true);
    SetBtn.attach(&setTimeModeISR, IRQ_FALL, 10, true);
    IncBtn.attach(&incTimeISR, IRQ_FALL, 10, true);

    while (1) {
        // Process log-time request.
        if (logTimeRequested) {
            logTimeToEEPROM();
            LCD.Clear(LCD_COLOR_WHITE);
            LCD.DisplayStringAt(0, 0, (uint8_t *)"Time Logged", CENTER_MODE);
            thread_sleep_for(500); // Brief confirmation display.
            logTimeRequested = false;
        }

        // Process view logs request.
        if (viewLogsRequested) {
            // Toggle view: if already in VIEW_LOGS, return to idle.
            if (system_state == VIEW_LOGS) {
                system_state = IDLE_MODE;
                LCD.Clear(LCD_COLOR_WHITE);
            } else {
                viewLogs();
            }
            viewLogsRequested = false;
        }

        // Process set-time mode request.
        if (setTimeModeRequested) {
            setTimeMode();
            setTimeModeRequested = false;
        }

        // Process increment request in set-time mode.
        if (incTimeRequested) {
            incrementTime();
            incTimeRequested = false;
        }

        // In idle mode, update the current time display.
        if (system_state == IDLE_MODE) {
            displayCurrentTime();
        }

        thread_sleep_for(1000); // Main loop delay.
    }
}

// Displays the current time in idle mode.
void displayCurrentTime() {
    time_t rawtime;
    time(&rawtime);
    char timebuf[30];
    strftime(timebuf, sizeof(timebuf), "%H:%M:%S", localtime(&rawtime));
    LCD.Clear(LCD_COLOR_WHITE);
    // Adjusted positions for Font12.
    LCD.DisplayStringAt(0, 0, (uint8_t *)"Time:", CENTER_MODE);
    LCD.DisplayStringAt(0, 12, (uint8_t *)timebuf, CENTER_MODE);
}

// Write data to EEPROM (remember: 63-byte write limit).
void WriteEEPROM(int address, unsigned int eeaddress, char *data, int size) {
    char i2cBuffer[size + 2];
    i2cBuffer[0] = (unsigned char)(eeaddress >> 8);  // MSB
    i2cBuffer[1] = (unsigned char)(eeaddress & 0xFF);  // LSB
    for (int i = 0; i < size; i++) {
        i2cBuffer[i + 2] = data[i];
    }
    int result = i2c.write(address, i2cBuffer, size + 2, false);
    thread_sleep_for(10); // Slightly increased delay to ensure EEPROM write completion.
}

// Read data from EEPROM.
void ReadEEPROM(int address, unsigned int eeaddress, char *data, int size) {
    char i2cBuffer[2];
    i2cBuffer[0] = (unsigned char)(eeaddress >> 8);  // MSB
    i2cBuffer[1] = (unsigned char)(eeaddress & 0xFF);  // LSB
    int result = i2c.write(address, i2cBuffer, 2, false);
    thread_sleep_for(10);
    i2c.read(address, data, size);
    thread_sleep_for(10);
}

// Logs the current time to EEPROM.
// It shifts the previous timestamp (EEPROM offset 10) to offset 0, then writes the new timestamp at offset 10.
void logTimeToEEPROM() {
    time_t rawtime;
    time(&rawtime);
    char timebuf[10];
    strftime(timebuf, sizeof(timebuf), "%H:%M:%S", localtime(&rawtime));

    // Shift the old log from offset 10 to offset 0.
    char oldTime[10] = {0};
    ReadEEPROM(EEPROM_ADDR, 10, oldTime, 10);
    WriteEEPROM(EEPROM_ADDR, 0, oldTime, 10);

    // Write the new log to offset 10.
    WriteEEPROM(EEPROM_ADDR, 10, timebuf, 10);
}

// Displays the logs stored in EEPROM.
void viewLogs() {
    char lastTime1[10] = {0}, lastTime2[10] = {0};

    // Read logs from EEPROM.
    ReadEEPROM(EEPROM_ADDR, 0, lastTime1, 10);
    ReadEEPROM(EEPROM_ADDR, 10, lastTime2, 10);

    LCD.Clear(LCD_COLOR_WHITE);
    LCD.SetFont(&Font12); 
    LCD.DisplayStringAt(0, 0, (uint8_t *)"Log Times:", CENTER_MODE);
    // Display the "Old" log on the next line.
    LCD.DisplayStringAt(0, 12, (uint8_t *)"Old:", LEFT_MODE);
    LCD.DisplayStringAt(0, 12, (uint8_t *)lastTime1, CENTER_MODE);
    // Display the "New" log on the following line.
    LCD.DisplayStringAt(0, 24, (uint8_t *)"New:", LEFT_MODE);
    LCD.DisplayStringAt(0, 24, (uint8_t *)lastTime2, CENTER_MODE);
    
    system_state = VIEW_LOGS;
}

// Called when the Set button is pressed.
// Cycles through time fields and exits set-time mode after the Year field.
void setTimeMode() {
    if (!inSetTimeMode) {
         inSetTimeMode = true;
         system_state = SET_TIME;
         selectedField = 0;
         time_t rawtime;
         time(&rawtime);
         newTime = *localtime(&rawtime);
    } else {
         if (selectedField == 5) {
             inSetTimeMode = false;
             system_state = IDLE_MODE;
             set_time(mktime(&newTime));
             displayCurrentTime();
             return;
         } else {
             selectedField++;
         }
    }
    
    char timebuf[40];
    sprintf(timebuf, "%04d-%02d-%02d %02d:%02d:%02d", 
            newTime.tm_year + 1900, newTime.tm_mon + 1, newTime.tm_mday,
            newTime.tm_hour, newTime.tm_min, newTime.tm_sec);
    LCD.Clear(LCD_COLOR_WHITE);
    LCD.DisplayStringAt(0, 0, (uint8_t *)"Set Time Mode", CENTER_MODE);
    LCD.DisplayStringAt(0, 10, (uint8_t *)timebuf, CENTER_MODE);

    char fieldLabel[30];
    switch(selectedField) {
        case 0: strcpy(fieldLabel, "Hour"); break;
        case 1: strcpy(fieldLabel, "Minute"); break;
        case 2: strcpy(fieldLabel, "Second"); break;
        case 3: strcpy(fieldLabel, "Day"); break;
        case 4: strcpy(fieldLabel, "Month"); break;
        case 5: strcpy(fieldLabel, "Year (SET exit)"); break;
        default: strcpy(fieldLabel, "Unknown"); break;
    }
    LCD.DisplayStringAt(0, 22, (uint8_t *)fieldLabel, CENTER_MODE);
}

// Called when the Inc button is pressed in set-time mode. Increments the selected field.
void incrementTime() {
    if (!inSetTimeMode) return;
    
    switch(selectedField) {
         case 0: newTime.tm_hour = (newTime.tm_hour + 1) % 24; break;
         case 1: newTime.tm_min = (newTime.tm_min + 1) % 60; break;
         case 2: newTime.tm_sec = (newTime.tm_sec + 1) % 60; break;
         case 3: newTime.tm_mday = (newTime.tm_mday % 31) + 1; break;
         case 4: newTime.tm_mon = (newTime.tm_mon + 1) % 12; break;
         case 5: newTime.tm_year += 1; break;
    }
    
    set_time(mktime(&newTime));
    
    char timebuf[40];
    sprintf(timebuf, "%04d-%02d-%02d %02d:%02d:%02d", 
            newTime.tm_year + 1900, newTime.tm_mon + 1, newTime.tm_mday,
            newTime.tm_hour, newTime.tm_min, newTime.tm_sec);
    LCD.Clear(LCD_COLOR_WHITE);
    LCD.DisplayStringAt(0, 0, (uint8_t *)"Set Time Mode", CENTER_MODE);
    LCD.DisplayStringAt(0, 10, (uint8_t *)timebuf, CENTER_MODE);

    char fieldLabel[30];
    switch(selectedField) {
        case 0: strcpy(fieldLabel, "Hour"); break;
        case 1: strcpy(fieldLabel, "Minute"); break;
        case 2: strcpy(fieldLabel, "Second"); break;
        case 3: strcpy(fieldLabel, "Day"); break;
        case 4: strcpy(fieldLabel, "Month"); break;
        case 5: strcpy(fieldLabel, "Year (SET exit)"); break;
        default: strcpy(fieldLabel, "Unknown"); break;
    }
    LCD.DisplayStringAt(0, 22, (uint8_t *)fieldLabel, CENTER_MODE);
} 