#include <FastLED.h>
#include <Rotary.h>
#include <Bounce2.h>
#include "pitches.h"

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define ENCODER_PIN_BUTTON 2
#define ENCODER_PIN_A 3
#define ENCODER_PIN_B 4
#define BUZZER_PIN 5
#define BUZZER_SWITCH_PIN 6
#define DATA_PIN      11
#define CLOCK_PIN     12
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
struct CRGB leds[NUM_LEDS];

uint8_t max_bright = 128;                                     // Overall brightness definition. It can be changed on the fly.

char logMessage[100];

int internalLedState = LOW;
int activeLedNumber = 0;
int redValue = 0;
int greenValue = 0;
int blueValue = 0;

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

boolean buzzerEnabled = false;
boolean redPressed = false;
boolean greenPressed = false;
boolean bluePressed = false;
boolean forwardPressed = false;
boolean backwardPressed = false;

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
        buttons[i].interval(25);                                   // interval in ms
    }

    buttons[BUZZER_SWITCH_ARRAY_POSITION].update();
    int buzzerState = buttons[BUZZER_SWITCH_ARRAY_POSITION].read();
    if ( buzzerState == LOW ) {
        buzzerEnabled = true;
    } else {
        buzzerEnabled = false;
    }
    
    playMelody();

    delay(1000); // power-up safety delay
    // It's important to set the color correction for your LED strip here,
    // so that colors can be more accurately rendered through the 'temperature' profiles
    FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(max_bright);
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

void resetColors() {
    redValue = 0;
    greenValue = 0;
    blueValue = 0;
}

void setColorRgb(int red, int green, int blue) {
    CRGB color = CRGB(red, green, blue);

    fill_solid(leds, NUM_LEDS, color);
}

void setColorHsv(int hue, int saturation, int value) {
    CHSV color = CHSV(hue, saturation, value);

    fill_solid(leds, NUM_LEDS, color);
}

void processButtonInputs() {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        // Update the Bounce instance :
        buttons[i].update();
        if ( buttons[i].fell() ) {
            switch (i) {
                case ENCODER_BUTTON_ARRAY_POSITION:
                    break;
                case RED_SWITCH_ARRAY_POSITION:
                    redPressed = false;
                    break;
                case GREEN_SWITCH_ARRAY_POSITION:
                    greenPressed = false;
                    break;
                case BLUE_SWITCH_ARRAY_POSITION:
                    bluePressed = false;
                    break;
                case FORWARD_SWITCH_ARRAY_POSITION:
                    forwardPressed = false;
                    break;
                case BACKWARD_SWITCH_ARRAY_POSITION:
                    backwardPressed = false;
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
            playBeep();
        }

        if ( buttons[i].rose() ) {
             switch (i) {
                case ENCODER_BUTTON_ARRAY_POSITION:
                    resetColors();
                    break;
                case RED_SWITCH_ARRAY_POSITION:
                    redPressed = true;
                    break;
                case GREEN_SWITCH_ARRAY_POSITION:
                    greenPressed = true;
                    break;
                case BLUE_SWITCH_ARRAY_POSITION:
                    bluePressed = true;
                    break;
                case FORWARD_SWITCH_ARRAY_POSITION:
                    forwardPressed = true;
                    break;
                case BACKWARD_SWITCH_ARRAY_POSITION:
                    
                    backwardPressed = true;
                    break;
                case BUZZER_SWITCH_ARRAY_POSITION:
                    Serial.println("Buzzer was disabled.");
                    buzzerEnabled = false;
                    break;
                default:
                    break;
            }
        }

        int currentButtonRead = buttons[i].read();
        if ( currentButtonRead == HIGH )
        {
            // buttons are not pressed
            switch (i) {
                case ENCODER_BUTTON_ARRAY_POSITION:
                    break;
                case RED_SWITCH_ARRAY_POSITION:
                    redPressed = false;
                    break;
                case GREEN_SWITCH_ARRAY_POSITION:
                    greenPressed = false;
                    break;
                case BLUE_SWITCH_ARRAY_POSITION:
                    bluePressed = false;
                    break;
                case FORWARD_SWITCH_ARRAY_POSITION:
                    forwardPressed = false;
                    break;
                case BACKWARD_SWITCH_ARRAY_POSITION:
                    break;
                case BUZZER_SWITCH_ARRAY_POSITION:
                    break;
                default:
                    break;
            }
        } else {
            // buttons are pressed
            switch (i) {
                case ENCODER_BUTTON_ARRAY_POSITION:
                    break;
                case RED_SWITCH_ARRAY_POSITION:
                    redPressed = true;
                    break;
                case GREEN_SWITCH_ARRAY_POSITION:
                    greenPressed = true;
                    break;
                case BLUE_SWITCH_ARRAY_POSITION:
                    bluePressed = true;
                    break;
                case FORWARD_SWITCH_ARRAY_POSITION:
                    forwardPressed = true;
                    break;
                case BACKWARD_SWITCH_ARRAY_POSITION:
                    backwardPressed = true;
                    break;
                case BUZZER_SWITCH_ARRAY_POSITION:
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
        playBeep();
        if (redPressed) {
            if ((redValue >= 0) && (redValue < 255)) {
                redValue += 1;
            } else {
                redValue = 255;
            }
        }
        if (greenPressed) {
            if ((greenValue >= 0) && (greenValue < 255)) {
                greenValue += 1;
            } else {
                greenValue = 255;
            }
        }
        if (bluePressed) {
            if ((blueValue >= 0) && (blueValue < 255)) {
                blueValue += 1;
            } else {
                blueValue = 255;
            }
        }
    }
    else if (result == DIR_CCW) {
        Serial.println("CounterClockWise");
        playBeep();
        if (redPressed) {
            if ((redValue > 0) && (redValue <= 255)) {
                redValue -= 1;
            } else {
                redValue = 0;
            }
        }
        if (greenPressed) {
            if ((greenValue > 0) && (greenValue <= 255)) {
                greenValue -= 1;
            } else {
                greenValue = 0;
            }
        }
        if (bluePressed) {
            if ((blueValue > 0) && (blueValue <= 255)) {
                blueValue -= 1;
            } else {
                blueValue = 0;
            }
        }
    }
}

void printValues(int firstValue, int secondValue, int thirdValue) {
    static int lastValue1 = 0;
    static int lastValue2 = 0;
    static int lastValue3 = 0;

    if ( (firstValue != lastValue1) || (secondValue != lastValue2) || (thirdValue != lastValue3))  {
        sprintf(logMessage, "Current values: forward %d, backward %d - first %d second %d third %d - Pressed keys: %d %d %d", forwardPressed, backwardPressed, firstValue, secondValue, thirdValue, redPressed, greenPressed, bluePressed);
        Serial.println(logMessage);

        // save the changed values
        lastValue1 = firstValue;
        lastValue2 = secondValue;
        lastValue3 = thirdValue;
    }
}

void loop()
{
    processButtonInputs();
    processClickWheelInputs();  

    if (forwardPressed) {
        setColorRgb(redValue, greenValue, blueValue);
    }
    else if (backwardPressed) {
        setColorHsv(redValue, greenValue, blueValue);
    }
    printValues(redValue, greenValue, blueValue);

    FastLED.show();
}
