#include "funzioni.h"

/*************************************************
*******************   DEFINE  ********************
*************************************************/

// LED
#define L4 10
#define L3 9
#define L2 8
#define L1 7
#define LS 6 // solo i pin con ~ permettono analog write

// TASTI
// Nel core Arduino, B0, B1, B2, … B7 sono macro predefinite che rappresentano i bit di una porta
#define BTN4 5
#define BTN3 4
#define BTN2 3
#define BTN1 2 // solo pin 2 e 3 permettono interrupt

// POTENZIOMETRO
#define POT_PIN A0

// DEBUG
#define MENU_DEBUG false

// TIMER
#define DEFAULT_MENU_TIMER 10'000UL // sintassi 10'000UL permessa da C++14/17
#define DEFAULT_GAME_TIMER 10'000UL
#define DEFAULT_GAME_OVER_TIMER 2'000UL
#define timerFactor 0.9

float gameLevel = 0;
float gameDifficulty;
unsigned long gameTimer = DEFAULT_GAME_TIMER;
float remaining;
GameState gameState; 
int sequenceIndex = 0;


/* Wiring: SDA => A4, SCL => A5 */
/* I2C address of the LCD: 0x27 */
/* Number of columns: 20 rows: 4 */

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,16,2);
int currentValue = 0;
int newValue = 0;
int b1_state;
int b2_state;
int b3_state;
int b4_state;

int fadeAmount;
int currIntensity;

volatile bool flagState = false;

boolean lsIsOn;

// Stato fade
volatile bool fadeActive = false;
volatile int  fadeValue  = 0;     // 0..255
volatile int  fadeStep   = 5;     // velocità del fade (incremento)
volatile bool tick       = false; // set da ISR → gestito nel loop
// volatile forza il compilatore a rileggere il valore direttamente dalla memoria RAM ogni volta che serve (quindi in seguito agli interrupt il loop() vede subito le modifiche).
volatile bool woke = false;  // opzionale, per sapere se c'è stato un wake

unsigned long lastTimer = 0;
String sequenza = "";


/*************************************************
******************    SETUP   ********************
*************************************************/

void setup() {
    Serial.begin(9600);

    lcdOn(lcd);
    // lcd.init();
    // lcd.backlight();
    

    // init Led
    pinMode(L4, OUTPUT);
    pinMode(L3, OUTPUT);
    pinMode(L2, OUTPUT);
    pinMode(L1, OUTPUT);
    pinMode(LS, OUTPUT);

    // init Tasti
    pinMode(BTN4, INPUT);
    pinMode(BTN3, INPUT);
    pinMode(BTN2, INPUT);
    // pinMode(BTN1, INPUT);
    pinMode(BTN1, INPUT_PULLUP);  // <--- NUOVO
    

    currIntensity = 0;
    fadeAmount = 20;

    gameState = GameState::BEGIN;

    lsIsOn = false;

    // usa una fonte "rumorosa" per non ripetere la sequenza ad ogni reset
    randomSeed(analogRead(A0) ^ micros());

}


/*************************************************
******************    LOOP    ********************
*************************************************/

void loop() {
    // printState(gameState);  
    switch (gameState) {
    case GameState::BEGIN:
        printLCD(lcd, "Welcome to TOS!", "Press B1 to Start");
        // Avvia lampeggio su LS (solo la prima volta)
        fadeOn(lsIsOn);
        applyFadeIfNeeded();

        if (timer(lastTimer, DEFAULT_MENU_TIMER, MENU_DEBUG)) { 
            gameState = GameState::SLEEP;
        }
        
        break;

        // ****************
        // ***** PLAY *****
        // ****************
        case GameState::PLAY: 
        fadeOff(LS, lsIsOn);
        if (sequenza == "") {
            sequenza = gen1234Str();
            gameTimer = (DEFAULT_GAME_TIMER * (pow(timerFactor, (gameLevel + gameDifficulty)))); // lvl e diff partono entrambi da 0
        }
        // Serial.println(sequenza);
        
        if (timer(lastTimer, gameTimer, MENU_DEBUG)) {
            sequenza = "";
            lastTimer = 0;
            gameLevel = 1;
            gameState = GameState::GAME_OVER;
        }
        
        remaining  = (gameTimer - (millis() - lastTimer)) / 1000.0;
        
        if (remaining >= 0 && remaining < 100)
            printLCD(lcd, "Sequenza " + String(sequenza), String(remaining, 1) + "s");
        break;




        case GameState::SLEEP:
        Serial.println("GOING TO SLEEP");
        fadeOff(LS, lsIsOn);
        Serial.flush();
        deepSleep();
        gameState = GameState::BEGIN;
        break;
        
        case GameState::GAME_OVER:
            printLCD(lcd, "Hai perso", "");
            digitalWrite(LS, HIGH);
            if (timer(lastTimer, DEFAULT_GAME_OVER_TIMER, MENU_DEBUG)) {
                lastTimer = 0;
                gameState = GameState::BEGIN;
        }
        break;

    default:
        break;
    }

    // BTN1 
    b1_state = digitalRead(BTN1);
    if (b1_state == HIGH) {
        delay(70);
        digitalWrite(L1, HIGH);
        Serial.println("BTN1");
        
        if (gameState == GameState::BEGIN) {
            lastTimer = 0; // reset timer inattivita'
            gameDifficulty = getDifficulty(POT_PIN);
            Serial.println("DIFF " + String(gameDifficulty));
            gameState = GameState::PLAY;
            delay(100);
        } else if (gameState == GameState::PLAY) {
            if ( ! checkSequence(sequenza, sequenceIndex, 1, gameLevel)) {
                gameState = GameState::GAME_OVER;
            }
        }
    } else {
        digitalWrite(L1, LOW);
    }

    // BTN2
    b2_state = digitalRead(BTN2);
    if (b2_state == HIGH) {
        delay(70);
        digitalWrite(L2, HIGH);
        Serial.println("BTN2");
        if (gameState == GameState::PLAY) {
            if ( ! checkSequence(sequenza, sequenceIndex, 2, gameLevel)) {
                
                gameState = GameState::GAME_OVER;
            }
        }
    } else {
        digitalWrite(L2, LOW);
    }

    // BTN3
    b3_state = digitalRead(BTN3);
    if (b3_state == HIGH) {
        delay(70);
        digitalWrite(L3, HIGH);
        Serial.println("BTN3");
        if (gameState == GameState::PLAY) {
            if ( ! checkSequence(sequenza, sequenceIndex, 3, gameLevel)) {
                gameState = GameState::GAME_OVER;
            }
        }
    } else {
        digitalWrite(L3, LOW);
    }

    // BTN4
    b4_state = digitalRead(BTN4);
    if (b4_state == HIGH) {
        delay(70);
        digitalWrite(L4, HIGH);
        Serial.println("BTN4");
        if (gameState == GameState::PLAY) {
            if ( ! checkSequence(sequenza, sequenceIndex, 4, gameLevel)) {
                gameState = GameState::GAME_OVER;
            }
        }
    } else {
        digitalWrite(L4, LOW);
    }
}

/*************************************************
******************  FUNZIONI  ********************
*************************************************/

bool checkSequence(String& sequenza, int& sequenceIndex, int input, float gameLevel) {
    delay(40);
    if ( sequenza[sequenceIndex] == ('0' + input)) { // converte int in char
        sequenceIndex++;
        if (sequenceIndex == 4) {
            sequenceIndex = 0;
            sequenza = "";
            gameLevel++;
        }
        Serial.println("giusto");
        return true;
    } else {
        sequenceIndex = 0;
        sequenza = "";
        gameLevel = 0;
        Serial.println("errore");
        return false;
    }
}








// controlla blink led rosso
void blinky() {
    flagState = !flagState;
    digitalWrite(LS, flagState ? HIGH : LOW);
}


// ISR chiamata dal Timer1 a intervalli regolari
void fadeISR() {
    if (!fadeActive) return;

    fadeValue += fadeStep;
    if (fadeValue >= 255) {         // inverti per scendere
        fadeValue = 255;
        fadeStep = -fadeStep;
    } else if (fadeValue <= 0) {    // inverti per salire
        fadeValue = 0;
        fadeStep = -fadeStep;
    }
    tick = true; // segnala al loop di aggiornare il PWM
}

// Aggiornamento PWM fuori dall'ISR
void applyFadeIfNeeded() {
    if (tick) {
        tick = false;
        // cast a variabile locale per evitare letture "spezzate" di volatile
        int v = fadeValue;
        
        // Curva quadratica (più morbida per l’occhio)
        int curved = (v * v) / 255;  
        analogWrite(LS, curved);
    }
}

// Accende il fade
void fadeOn(boolean &lsIsOn) {
    if (!lsIsOn) {
        lsIsOn = true;
        pinMode(LS, OUTPUT);
        fadeActive = true;
        fadeValue  = 0;
        fadeStep   = 5;

        // Imposta la frequenza di aggiornamento del fade:
        // 5000 µs = 5 ms → ~200 aggiornamenti al secondo
        Timer1.initialize(5000);
        Timer1.attachInterrupt(fadeISR);
    }
}

// Ferma il fade e spegne il LED
void fadeOff(int led, boolean &lsIsOn) {
    if (lsIsOn) {
        lsIsOn = false;
        fadeActive = false;
        Timer1.detachInterrupt();
        analogWrite(led, 0);          // LED off
        digitalWrite(led, LOW);
    }
}

void lightSleep(void) {
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();
    power_adc_disable();
    power_spi_disable();
    power_timer0_disable();
    power_timer1_disable();
    power_timer2_disable();
    power_twi_disable();
    sleep_mode();
    /* back */
    sleep_disable();
    power_all_enable();
}



// Funzione di deep sleep
void deepSleep() {
    // BTN1 ha già INPUT_PULLUP impostato in setup()
    // se il pulsante wake è premuto (LOW), aspetta il rilascio prima di dormire
    if (digitalRead(BTN1) == LOW) { // TODO ci deve andare HIGH
        Serial.println("BTN1 premuto");
        // while (digitalRead(BTN1) == LOW) { /* wait release */ }
        // delay(20); // piccolo debounce
    }

    // ferma l’eventuale fade/Timer1 prima di spegnere i timer
    Timer1.detachInterrupt();
    fadeActive = false;


    // *** SPEGNI USCITE ***
    turnOffAllLeds();
    // *** SPEGNI LCD *** (fallo PRIMA di spegnere TWI/I2C)
    lcdOff(lcd);
    

    // disattiva periferiche
    power_adc_disable();
    power_spi_disable();
    power_timer0_disable();
    power_timer1_disable();
    power_timer2_disable();
    power_twi_disable();

    // disabilita pin-change interrupts e pulisci flag pendenti
    PCICR = 0;
    PCIFR |= (1 << PCIF0) | (1 << PCIF1) | (1 << PCIF2); // clear PCINT
    EIFR  |= (1 << INTF0);                               // clear INT0

    // wake su livello LOW di INT0 (D2/BTN1)
    attachInterrupt(digitalPinToInterrupt(BTN1), wakeUpISR, LOW);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    noInterrupts();
    sleep_enable();
    sleep_bod_disable();   // consumi minimi
    interrupts();

    sleep_cpu();           // --- qui dorme davvero ---

    // al risveglio
    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(BTN1));
    power_all_enable();
    

    // anti-rientro: attendi rilascio BTN1 se ancora basso
    delay(30);
    while (digitalRead(BTN1) == LOW) { /* wait */ }
    lcdOn(lcd);
}


// helper per spegnere tutti i LED
inline void turnOffAllLeds() {
    analogWrite(LS, 0);     // se stava usando PWM
    digitalWrite(LS, LOW);
    digitalWrite(L1, LOW);
    digitalWrite(L2, LOW);
    digitalWrite(L3, LOW);
    digitalWrite(L4, LOW);
}

