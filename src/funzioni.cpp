#include "funzioni.h"


// Stampa sullo schermo LCD senza clear()
void printLCD(LiquidCrystal_I2C lcd, String riga1, String riga2) {
    const unsigned int maxLcdLen = 16;
    char c;
    uint32_t now = millis();
    static uint32_t last = 0;

    if (now - last < 100) return;   // piccolo throttle anti-flicker

    lcd.setCursor(0, 0);
    for (unsigned int i = 0; i < maxLcdLen; i++) {
        c = (i < riga1.length()) ? riga1[i] : ' ';
        lcd.write(c);
    }

    lcd.setCursor(0, 1);
    for (unsigned int i = 0; i < maxLcdLen; i++) {
        c = (i < riga2.length()) ? riga2[i] : ' ';
        lcd.write(c);
    }
    last = now;
}


// controlla blink led rosso


// inizia a lampeggiare
void blinkOn(boolean& lson) {
    if (!lson) {
        lson = true;
        Timer1.initialize(500000);      // 0.5s → 2Hz
        Timer1.attachInterrupt(blinky); // chiama ISR senza parametri
    }
}

// smetti di lampeggiare
void blinkOff(int led, boolean& lson) {
    if (lson) {
        lson = false;
        Timer1.detachInterrupt(); // stop timer
        digitalWrite(led, LOW);
    }
}

void printState(GameState gameState) {
    switch (gameState)
    {
    case GameState::BEGIN:
        Serial.println("Game State: BEGIN");
        break;
    
        case GameState::PLAY:
        Serial.println("Game State: PLAY");
        break;

        case GameState::SLEEP:
        Serial.println("Game State: SLEEP");
        break;

    default:
        break;
    }
};

String gen1234Str() {
    uint8_t a[4] = {1, 2, 3, 4};

    // Fisher–Yates shuffle
    for (int i = 3; i > 0; --i) {
        int j = random(i + 1); // 0..i
        uint8_t t = a[i]; a[i] = a[j]; a[j] = t;
    }

    char buf[5];
    buf[0] = '0' + a[0];
    buf[1] = '0' + a[1];
    buf[2] = '0' + a[2];
    buf[3] = '0' + a[3];
    buf[4] = '\0';

    return String(buf);
}

// ISR di risveglio (vuota ma necessaria)
void wakeUpISR() {
  // Non fare nulla qui: basta che l’interrupt avvenga
}


float getDifficulty(int potPin) {
    return (float) map(analogRead(potPin), 0, 1023, 0, 4);
}


// true se sono passati x secondi, quando sono passati resetta last
bool timer(unsigned long& last, unsigned long interval, bool debug) {
    unsigned long now = millis();
    unsigned long elapsed = (last == 0) ? 0UL : (now - last);


    if (debug) {
        Serial.print(F("[timer] elapsed="));
        Serial.print(elapsed);
        Serial.print(F(" ms / target="));
        Serial.print(interval);
        Serial.println(F(" ms"));
    }
    // Prima chiamata: armo il timer e non scatto subito
    if (last == 0) { 
        last = now; 
        return false; 
    }

    // Controllo
    if (now - last >= interval) {
        last = 0;
        if (debug) Serial.println(F("[timer] TRIGGER"));
        return true;
    }
    return false;
}

void lcdOn(LiquidCrystal_I2C& lcd) {
    lcd.init();
    lcd.backlight();
}

void lcdOff(LiquidCrystal_I2C& lcd) {
    lcd.noDisplay();
    lcd.noBacklight();
}
