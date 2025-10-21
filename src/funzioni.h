#ifndef FUNZIONI_H
#define FUNZIONI_H
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "TimerOne.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>

// STATI
enum class GameState {
    BEGIN = 1,
    PLAY,
    SLEEP,
    GAME_OVER
};

void printLCD(LiquidCrystal_I2C lcd, String riga1, String riga2 = "");
void blinky();
void blinkOn(boolean& lson);
void blinkOff(int led, boolean& lson);
void fadeOff(int led, boolean &lson);
void fadeISR();
void applyFadeIfNeeded();
void fadeOn(boolean &lson);
void lightSleep();
void wakeUpISR();
void deepSleep();
void printState(GameState gameState);
inline void turnOffAllLeds();
String gen1234Str();
float getDifficulty(int potPin);
bool timer(unsigned long& last, unsigned long interval, bool debug=false);
void lcdOn(LiquidCrystal_I2C& lcd);
void lcdOff(LiquidCrystal_I2C& lcd);
bool checkSequence(String& sequenza, int& sequenceIndex, int input, float gameLevel);






#endif // FUNZIONI_H