#include <FastLED.h>
#include <Rotary.h>
#include <Bounce2.h>
#include "pitches.h"

#define ENCODER_PIN_BUTTON 2
#define ENCODER_PIN_A 3
#define ENCODER_PIN_B 4
#define BUZZER_PIN 5
#define BUZZER_SWITCH_PIN 6
#define DATA_PIN      11
#define CLOCK_PIN     12
#define LED_PIN       13 // Its the internal LED on the Arduino
#define BUTTON_RED_PIN 22
#define BUTTON_GREEN_PIN 24
#define BUTTON_BLUE_PIN 26
#define FORWARD_SWITCH_PIN 28
#define BACKWARD_SWITCH_PIN 30
// #define ROTATION_SWITCH_POS_1_PIN 32
// #define ROTATION_SWITCH_POS_2_PIN 34
// #define ROTATION_SWITCH_POS_3_PIN 36
// #define ROTATION_SWITCH_POS_4_PIN 38
// #define ROTATION_SWITCH_POS_5_PIN 40
// #define ROTATION_SWITCH_POS_6_PIN 42
#define ENCODER_BUTTON_ARRAY_POSITION 0
#define RED_SWITCH_ARRAY_POSITION 1
#define GREEN_SWITCH_ARRAY_POSITION 2
#define BLUE_SWITCH_ARRAY_POSITION 3
#define FORWARD_SWITCH_ARRAY_POSITION 4
#define BACKWARD_SWITCH_ARRAY_POSITION 5
#define BUZZER_SWITCH_ARRAY_POSITION 6


#define NUM_BUTTONS 7
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {ENCODER_PIN_BUTTON, BUTTON_RED_PIN, BUTTON_GREEN_PIN, BUTTON_BLUE_PIN,
 FORWARD_SWITCH_PIN, BACKWARD_SWITCH_PIN, BUZZER_SWITCH_PIN};


// Information about the LED strip itself
#define NUM_LEDS    12
#define CHIPSET     WS2801
#define COLOR_ORDER RGB
CRGB leds[NUM_LEDS];

#define BRIGHTNESS  128

int internalLedState = LOW;
int activeLedNumber = 0;
int ledInc = 1;
int ledSpeed = 10;

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

boolean buzzerEnabled = false;
boolean needToToggleLed = false;

Rotary r = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);
Bounce * buttons = new Bounce[NUM_BUTTONS];

// declaring prototypes
void playMelody();
void playBeep();

void setup() {
    Serial.begin(115200);
    r.begin(true);

    for (int i = 0; i < NUM_BUTTONS; i++) {
        buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP  );       //setup the bounce instance for the current button
        buttons[i].interval(25);              // interval in ms
    }

    buttons[BUZZER_SWITCH_ARRAY_POSITION].update();
    int buzzerState = buttons[BUZZER_SWITCH_ARRAY_POSITION].read();
    if ( buzzerState == LOW ) {
        buzzerEnabled = true;
    } else {
        buzzerEnabled = false;
    }
    
    pinMode(LED_PIN,OUTPUT); // Setup the LED
    digitalWrite(LED_PIN,internalLedState);
    playMelody();

    delay(3000); // power-up safety delay
    delay(1000); // power-up safety delay
    // It's important to set the color correction for your LED strip here,
    // so that colors can be more accurately rendered through the 'temperature' profiles
    FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS)
        .setCorrection( Typical8mmPixel )
        .setDither(BRIGHTNESS < 255);
    FastLED.setBrightness( BRIGHTNESS );
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);               // FastLED Power management set at 5V, 500mA.
}

void playMelody() {
    if (buzzerEnabled) {
        // iterate over the notes of the melody:    
        for (int thisNote = 0; thisNote < 8; thisNote++) {

            // to calculate the note duration, take one second divided by the note type.
            //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
            int noteDuration = 1000 / noteDurations[thisNote];
            tone(BUZZER_PIN, melody[thisNote], noteDuration);

            // to distinguish the notes, set a minimum time between them.
            // the note's duration + 30% seems to work well:
            int pauseBetweenNotes = noteDuration * 1.30;
            delay(pauseBetweenNotes);
            // stop the tone playing:
            noTone(BUZZER_PIN);
        }
    }
}

void playBeep() {
    if (buzzerEnabled) {
        tone(BUZZER_PIN, 1000, 100);
    }
}

void processButtonInputs() {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        // Update the Bounce instance :
        buttons[i].update();
        // If it fell, flag the need to toggle the LED
        if ( buttons[i].fell() ) {
            switch (i) {
                case ENCODER_BUTTON_ARRAY_POSITION:
                    break;
                case RED_SWITCH_ARRAY_POSITION:
                    break;
                case GREEN_SWITCH_ARRAY_POSITION:
                    break;
                case BLUE_SWITCH_ARRAY_POSITION:
                    break;
                case FORWARD_SWITCH_ARRAY_POSITION:
                    break;
                case BACKWARD_SWITCH_ARRAY_POSITION:
                    break;
                case BUZZER_SWITCH_ARRAY_POSITION:
                    Serial.println("Buzzer was enabled.");
                    buzzerEnabled = true;
                    break;
                default:
                    break;
            }

            String message = "Button on Pin " + BUTTON_PINS[i];
            message = message + " was pressed";
            Serial.println(message);
            needToToggleLed = true;
            ledInc *= -1;
            playBeep();
        }

        if ( buttons[i].rose() ) {
             switch (i) {
                case ENCODER_BUTTON_ARRAY_POSITION:
                    break;
                case RED_SWITCH_ARRAY_POSITION:
                    break;
                case GREEN_SWITCH_ARRAY_POSITION:
                    break;
                case BLUE_SWITCH_ARRAY_POSITION:
                    break;
                case FORWARD_SWITCH_ARRAY_POSITION:
                    break;
                case BACKWARD_SWITCH_ARRAY_POSITION:
                    break;
                case BUZZER_SWITCH_ARRAY_POSITION:
                    Serial.println("Buzzer was disabled.");
                    buzzerEnabled = false;
                    break;
                default:
                    break;
            }
        }
    }
}

void processClickWheelInputs() {
    unsigned char result = r.process();
    if (result == DIR_NONE) {
        // do nothing
    }
    else if (result == DIR_CW) {
        Serial.println("ClockWise");
        ledSpeed = ledSpeed + 1;
        playBeep();
    }
    else if (result == DIR_CCW) {
        Serial.println("CounterClockWise");
        ledSpeed = ledSpeed - 1;

        if (ledSpeed <= 1) {
            ledSpeed = 1;
        }
        playBeep();
    }
}

void loop()
{
    FastLED.clear();
    for (int i = 0; i < NUM_LEDS; i++) {
        if (i == activeLedNumber) {
            leds[i] = CRGB::Green;
        } else {
            leds[i] = CRGB::Blue;
        }
    }
    
    FastLED.show();
    
    for (int t = 0; t < NUM_LEDS; t++) {
        FastLED.delay(ledSpeed);

        processButtonInputs();
        processClickWheelInputs();

        // if a LED toggle has been flagged :
        if (needToToggleLed) {
            // Toggle LED state :
            internalLedState = !internalLedState;
            digitalWrite(LED_PIN, internalLedState);
        }
    }

    activeLedNumber = (activeLedNumber + ledInc) % NUM_LEDS;
}
