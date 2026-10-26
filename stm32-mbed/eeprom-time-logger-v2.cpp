#include "LCD_DISCO_F429ZI.h"
#include "fonts.h"
#include "mbed.h"
#include <time.h>
#include <cstdio>
#include <cstring>
#include "DebouncedInterrupt.h"

#define EEPROM_I2C_SDA_PIN        PC_9
#define EEPROM_I2C_SCL_PIN        PA_8
#define EEPROM_I2C_ADDRESS        0xA0
#define EEPROM_PREV_LOG_OFFSET    0
#define EEPROM_LATEST_LOG_OFFSET  10

LCD_DISCO_F429ZI LCD;
I2C eepromI2cBus(EEPROM_I2C_SDA_PIN, EEPROM_I2C_SCL_PIN);

// Button configuration
InterruptIn timeLogButton(BUTTON1); // User button for logging time
DebouncedInterrupt logViewToggleButton(PA_5); // External button to toggle log view
DebouncedInterrupt modeSelectButton(PA_6); // Button to enter/save time setting mode
DebouncedInterrupt incrementButton(PA_7); // Button to increment selected time field

// System state management
enum SystemState {
    DISPLAY_TIME_MODE,
    LOG_VIEW_MODE,
    TIME_SET_MODE
};
SystemState currentSystemState = DISPLAY_TIME_MODE;

// Time setting mode variables
tm pendingTimeUpdate;
int selectedTimeField = 0; // 0-5: hour, minute, second, day, month, year
bool timeSettingActive = false;

// Event flags set by interrupt handlers
volatile bool timeLogEventFlag = false;
volatile bool logViewEventFlag = false;
volatile bool modeSelectEventFlag = false;
volatile bool incrementEventFlag = false;

void writeToEeprom(int deviceAddress, unsigned int memoryAddress, char* dataBuffer, int dataSize);
void readFromEeprom(int deviceAddress, unsigned int memoryAddress, char* dataBuffer, int dataSize);
void logCurrentTimeToEeprom();
void displayLoggedTimes();
void handleTimeSettingMode();
void adjustSelectedTimeField();
void updateDisplayWithCurrentTime();

//Set ISR flags
void handleTimeLogInterrupt() {
    timeLogEventFlag = true;
}

void handleLogViewInterrupt() {
    logViewEventFlag = true;
}

void handleModeSelectInterrupt() {
    modeSelectEventFlag = true;
}

void handleIncrementInterrupt() {
    incrementEventFlag = true;
}

int main() {
    LCD.SetFont(&Font16);
    LCD.SetTextColor(LCD_COLOR_BLACK);

    // Initialize clock with default time (Jan 1, 2025 00:00:00)
    tm initialTime = {
        .tm_sec = 0,
        .tm_min = 0,
        .tm_hour = 0,
        .tm_mday = 1,
        .tm_mon = 0,
        .tm_year = 125
    };
    set_time(mktime(&initialTime));

    // Configure button interrupts
    timeLogButton.fall(&handleTimeLogInterrupt);
    logViewToggleButton.attach(&handleLogViewInterrupt, IRQ_FALL, 10, true);
    modeSelectButton.attach(&handleModeSelectInterrupt, IRQ_FALL, 10, true);
    incrementButton.attach(&handleIncrementInterrupt, IRQ_FALL, 10, true);

    while (true) {
        // Handle time logging requests
        if (timeLogEventFlag) {
            logCurrentTimeToEeprom();
            LCD.Clear(LCD_COLOR_WHITE);
            LCD.DisplayStringAt(0, 40, (uint8_t *)"Time Logged", CENTER_MODE);
            thread_sleep_for(500);
            timeLogEventFlag = false;
        }

        // Handle log view toggle requests
        if (logViewEventFlag) {
            if (currentSystemState == LOG_VIEW_MODE) {
                currentSystemState = DISPLAY_TIME_MODE;
                LCD.Clear(LCD_COLOR_WHITE);
            } else {
                displayLoggedTimes();
            }
            logViewEventFlag = false;
        }

        // Handle time setting mode requests
        if (modeSelectEventFlag) {
            LCD.SetFont(&Font16);
            handleTimeSettingMode();
            modeSelectEventFlag = false;
        }

        // Handle time increment requests
        if (incrementEventFlag) {
            adjustSelectedTimeField();
            incrementEventFlag = false;
        }

        // Update main display when in default mode
        if (currentSystemState == DISPLAY_TIME_MODE) {
            LCD.SetFont(&Font24);
            updateDisplayWithCurrentTime();
        }

        thread_sleep_for(1000);
    }
}

void updateDisplayWithCurrentTime() {
    time_t currentTimestamp;
    time(&currentTimestamp);
    char timeBuffer[30];
    strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", localtime(&currentTimestamp));
   
    LCD.Clear(LCD_COLOR_WHITE);
    LCD.DisplayStringAt(0, 40, (uint8_t *)"Current Time:", CENTER_MODE);
    LCD.DisplayStringAt(0, 80, (uint8_t *)timeBuffer, CENTER_MODE);
}

void writeToEeprom(int deviceAddress, unsigned int memoryAddress, char* dataBuffer, int dataSize) {
    const int addressHeaderSize = 2;
    char i2cBuffer[dataSize + addressHeaderSize];
   
    i2cBuffer[0] = (memoryAddress >> 8) & 0xFF;   // High address byte
    i2cBuffer[1] = memoryAddress & 0xFF;          // Low address byte
    memcpy(&i2cBuffer[addressHeaderSize], dataBuffer, dataSize);
   
    eepromI2cBus.write(deviceAddress, i2cBuffer, dataSize + addressHeaderSize);
    thread_sleep_for(15);
}

void readFromEeprom(int deviceAddress, unsigned int memoryAddress, char* dataBuffer, int dataSize) {
    char addressBuffer[2] = {
        static_cast<char>((memoryAddress >> 8) & 0xFF),
        static_cast<char>(memoryAddress & 0xFF)
    };
   
    eepromI2cBus.write(deviceAddress, addressBuffer, 2);
    thread_sleep_for(15);
    eepromI2cBus.read(deviceAddress, dataBuffer, dataSize);
    thread_sleep_for(15);
}

void logCurrentTimeToEeprom() {
    time_t currentTimestamp;
    time(&currentTimestamp);
    char timeBuffer[10];
    strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", localtime(&currentTimestamp));

    // Shift previous log to historical storage
    char previousLog[10] = {0};
    readFromEeprom(EEPROM_I2C_ADDRESS, EEPROM_LATEST_LOG_OFFSET, previousLog, 10);
    writeToEeprom(EEPROM_I2C_ADDRESS, EEPROM_PREV_LOG_OFFSET, previousLog, 10);

    // Store new log in latest position
    writeToEeprom(EEPROM_I2C_ADDRESS, EEPROM_LATEST_LOG_OFFSET, timeBuffer, 10);
}

void displayLoggedTimes() {
    char previousLogTime[10] = {0};
    char latestLogTime[10] = {0};

    readFromEeprom(EEPROM_I2C_ADDRESS, EEPROM_PREV_LOG_OFFSET, previousLogTime, 10);
    readFromEeprom(EEPROM_I2C_ADDRESS, EEPROM_LATEST_LOG_OFFSET, latestLogTime, 10);

    LCD.Clear(LCD_COLOR_WHITE);
    LCD.SetFont(&Font16);
    LCD.DisplayStringAt(0, 40, (uint8_t *)"Logged Times:", CENTER_MODE);
    LCD.DisplayStringAt(0, 80, (uint8_t *)"Previous:", LEFT_MODE);
    LCD.DisplayStringAt(0, 80, (uint8_t *)previousLogTime, CENTER_MODE);
    LCD.DisplayStringAt(0, 120, (uint8_t *)"Latest:", LEFT_MODE);
    LCD.DisplayStringAt(0, 120, (uint8_t *)latestLogTime, CENTER_MODE);
   
    currentSystemState = LOG_VIEW_MODE;
}

void handleTimeSettingMode() {
    if (!timeSettingActive) {
        timeSettingActive = true;
        currentSystemState = TIME_SET_MODE;
        selectedTimeField = 0;
        time_t currentTimestamp;
        time(&currentTimestamp);
        pendingTimeUpdate = *localtime(&currentTimestamp);
    } else {
        if (selectedTimeField == 5) {
            timeSettingActive = false;
            currentSystemState = DISPLAY_TIME_MODE;
            set_time(mktime(&pendingTimeUpdate));
            updateDisplayWithCurrentTime();
            return;
        } else {
            selectedTimeField++;
        }
    }

    char formattedTime[40];
    snprintf(formattedTime, sizeof(formattedTime), "%04d-%02d-%02d %02d:%02d:%02d",
            pendingTimeUpdate.tm_year + 1900, pendingTimeUpdate.tm_mon + 1,
            pendingTimeUpdate.tm_mday, pendingTimeUpdate.tm_hour,
            pendingTimeUpdate.tm_min, pendingTimeUpdate.tm_sec);

    LCD.Clear(LCD_COLOR_WHITE);
    LCD.DisplayStringAt(0, 40, (uint8_t *)"Time Setting Mode", CENTER_MODE);
    LCD.DisplayStringAt(0, 80, (uint8_t *)formattedTime, CENTER_MODE);

    const char* fieldLabels[] = {
        "Adjust Hour",
        "Adjust Minute",
        "Adjust Second",
        "Adjust Day",
        "Adjust Month",
        "Adjust Year (Exit)"
    };
    LCD.DisplayStringAt(0, 120, (uint8_t *)fieldLabels[selectedTimeField], CENTER_MODE);
}

void adjustSelectedTimeField() {
    if (!timeSettingActive) return;

    switch (selectedTimeField) {
        case 0:  // Hour
            pendingTimeUpdate.tm_hour = (pendingTimeUpdate.tm_hour + 1) % 24;
            break;
        case 1:  // Minute
            pendingTimeUpdate.tm_min = (pendingTimeUpdate.tm_min + 1) % 60;
            break;
        case 2:  // Second
            pendingTimeUpdate.tm_sec = (pendingTimeUpdate.tm_sec + 1) % 60;
            break;
        case 3:  // Day
            pendingTimeUpdate.tm_mday = (pendingTimeUpdate.tm_mday % 31) + 1;
            break;
        case 4:  // Month
            pendingTimeUpdate.tm_mon = (pendingTimeUpdate.tm_mon + 1) % 12;
            break;
        case 5:  // Year
            pendingTimeUpdate.tm_year++;
            break;
    }

    // Update RTC immediately for feedback
    set_time(mktime(&pendingTimeUpdate));

    char formattedTime[40];
    snprintf(formattedTime, sizeof(formattedTime), "%04d-%02d-%02d %02d:%02d:%02d",
            pendingTimeUpdate.tm_year + 1900, pendingTimeUpdate.tm_mon + 1,
            pendingTimeUpdate.tm_mday, pendingTimeUpdate.tm_hour,
            pendingTimeUpdate.tm_min, pendingTimeUpdate.tm_sec);

    LCD.Clear(LCD_COLOR_WHITE);
    LCD.DisplayStringAt(0, 40, (uint8_t *)"Time Setting Mode", CENTER_MODE);
    LCD.DisplayStringAt(0, 80, (uint8_t *)formattedTime, CENTER_MODE);

    const char* fieldLabels[] = {
        "Adjust Hour",
        "Adjust Minute",
        "Adjust Second",
        "Adjust Day",
        "Adjust Month",
        "Adjust Year (Exit)"
    };
    LCD.DisplayStringAt(0, 120, (uint8_t *)fieldLabels[selectedTimeField], CENTER_MODE);
}