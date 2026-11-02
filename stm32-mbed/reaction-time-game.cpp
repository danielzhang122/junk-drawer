#include "LCD_DISCO_F429ZI.h"
#include "mbed.h"

LCD_DISCO_F429ZI LCD;

InterruptIn userButton(BUTTON1);
InterruptIn external_button(PA_6, PullUp);

DigitalOut led3(PG_13); // Green LED

void toggle_led3() { led3 = !led3; }

bool cheating = false; //booleans for cheating/early input
bool too_early = false;
bool timing = false; //boolean for timing the reaction
bool waiting_for_led = false;

Ticker ticker_1;
Timer timer1;
Timeout timeout_1;

uint32_t elapsed_ms; //initiializing the time
uint32_t high_score = 2147483647; //highest value integer initially (will be replaced)

void TurnOn() { //turns on, starts timer for user to press
    led3 = 1;
    timer1.start();
    timing = true;
    waiting_for_led = false;
}

void stopTiming() {
    timer1.stop();
    elapsed_ms = chrono::duration_cast<chrono::milliseconds>(timer1.elapsed_time()).count(); //calculate elapsed time
    
    if (elapsed_ms == 0) { //cheating
        cheating = true;
    }
    if (elapsed_ms < high_score && !cheating) { //if calculated score is lowest, then it becomes high score
        high_score = elapsed_ms;
    }
    
    // Reset to flickering state
    timer1.reset();
    timing = false;
    led3 = 0;
    ticker_1.attach(&toggle_led3, 100ms);
}

void start() {
    if (!timing && !waiting_for_led) { //if timing and waiting for led arent true,
        // Reset flags for new attempt
        cheating = false;
        too_early = false;
        
        waiting_for_led = true; //start the waiting phase
        ticker_1.detach();
        led3 = 0;
        float randTime = rand() % 5 + 1;
        timeout_1.attach(&TurnOn, randTime); //turns on at a random time between five seconds after waiting_for_led is true
    }
}

void buttonHandler() {
    if (timing) { //if pressed when timing is true, then it will stop timing
        stopTiming();
    } else {
        if (waiting_for_led) { //if button is pressed when waiting_for_led is true, then it is too early
            // Handle early press
            too_early = true;
            timeout_1.detach();
            waiting_for_led = false;
            led3 = 0;
            ticker_1.attach(&toggle_led3, 100ms);
        } else {
            start(); //starts the timer when the waiting period is over
        }
    }
}

void reset_everything() {
    // Cancel any ongoing operations
    timeout_1.detach();
    timer1.stop();
    timer1.reset();
    
    // Reset all states and scores
    elapsed_ms = 0;
    high_score = 2147483647;
    timing = false;
    cheating = false;
    too_early = false;
    waiting_for_led = false;
    
    // Return to initial flickering state
    led3 = 0;
    ticker_1.attach(&toggle_led3, 100ms);
}

int main() {
    srand(time(0)); //seed the timer so different values pop up each time u run


    userButton.fall(&buttonHandler); //button input
    external_button.fall(&reset_everything); //external button input(breadboard)

    LCD.SetFont(&Font24);
    LCD.SetTextColor(LCD_COLOR_DARKBLUE);
    ticker_1.attach(&toggle_led3, 100ms);

    while (1) { //display
        LCD.Clear(LCD_COLOR_WHITE);
        uint8_t text1[30], text2[30], text3[30], text4[30];
        
        LCD.DisplayStringAt(0, 40, (uint8_t *)"Reaction Times", LEFT_MODE);
        sprintf((char *)text1, "Latest: %lu ms", elapsed_ms); //display score
        sprintf((char *)text2, "Best: %lu ms", high_score); //display high score
        LCD.DisplayStringAt(0, 80, text1, LEFT_MODE);
        LCD.DisplayStringAt(0, 120, text2, LEFT_MODE);
        
        if (cheating) {
            sprintf((char *)text3, "cheater"); //display "cheater" if reaction time is 0
            LCD.DisplayStringAt(0, 160, text3, LEFT_MODE);
        } else if (too_early) {
            sprintf((char *)text4, "too early"); //display "too early" if user clicked the button too early
            LCD.DisplayStringAt(0, 160, text4, LEFT_MODE);
        }
        
        thread_sleep_for(1000);
    }
}