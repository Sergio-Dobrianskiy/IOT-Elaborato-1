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
void blinkOn(boolean& lson);
void blinkOff(int led, boolean& lson);
void fadeOff(int led, boolean &lsIsOn, volatile bool& fadeActive);
void fadeISR();
void applyFadeIfNeeded(int led, int fadeValue, volatile bool& tick);
void fadeOn(boolean &lson);
void wakeUpISR();
void deepSleep();
void printState(GameState gameState);
inline void turnOffAllLeds();
String gen1234Str();
float getDifficulty(int potPin);
bool timer(unsigned long& last, unsigned long interval);
void lcdOn(LiquidCrystal_I2C& lcd);
void lcdOff(LiquidCrystal_I2C& lcd);
bool checkSequence(String& sequenza, int& sequenceIndex, int input, float& gameLevel);
void blinky();





#endif // FUNZIONI_H