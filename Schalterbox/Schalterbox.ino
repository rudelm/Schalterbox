/*
 * Contains code from one_sine_pal by Andrew Tuline
 * https://github.com/atuline/FastLED-Demos/blob/master/one_sine_pal/one_sine_pal.ino
 * 
 * Forget using delay() statements or counting pixels up and down the strand of LED's. This example demonstrates just a bit of what can be done using basic high school math
 * by using a simple sinewave.
 * 
 * You can use a sine wave to go back and forth up a strand of LED's. You can shift the phase of a sine wave to move along a strand. You can clip the top of that sine wave
 * to just display a certain value (or greater). You can add palettes to that sine wave so that you aren't stuck with CHSV values and so on.
 * 
 * It shows that you don't neeed a pile of for loops, delays or counting pixels in order to come up with a decent animation.
 */

#include <FastLED.h>
#include <Rotary.h>
#include <Bounce2.h>
#include "pitches.h"

// Use qsuba for smooth pixel colouring and qsubd for non-smooth pixel colouring
#define qsubd(x, b)  ((x>b)?b:0)                            // Digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b)  ((x>b)?x-b:0)                          // Analog Unsigned subtraction macro. if result <0, then => 0

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

// Initialize changeable global variables. Play around with these!!!
int8_t thisspeed = 8;                                         // You can change the speed of the wave, and use negative values.
uint8_t allfreq = 32;                                         // You can change the frequency, thus distance between bars.
int thisphase = 0;                                            // Phase change value gets calculated.
uint8_t thiscutoff = 192;                                     // You can change the cutoff value to display this wave. Lower value = longer wave.
int thisdelay = 30;                                           // You can change the delay. Also you can change the allspeed variable above. 
uint8_t bgclr = 0;                                            // A rotating background colour.
uint8_t bgbright = 10;                                        // Brightness of background colour

// Palette definitions
CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
TBlendType    currentBlending;

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
    
    playMelody();

    delay(1000); // power-up safety delay
    // It's important to set the color correction for your LED strip here,
    // so that colors can be more accurately rendered through the 'temperature' profiles
    FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS)
        .setCorrection( Typical8mmPixel );
    FastLED.setBrightness(max_bright);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);               // FastLED Power management set at 5V, 500mA.

    currentPalette = LavaColors_p;
    currentBlending = LINEARBLEND;
}

void one_sine_pal(uint8_t colorIndex) {                                       // This is the heart of this program. Sure is short.
  
  thisphase += thisspeed;                                                     // You can change direction and speed individually.
  
  for (int k=0; k<=NUM_LEDS-1; k++) {                                          // For each of the LED's in the strand, set a brightness based on a wave as follows:
    int thisbright = qsubd(cubicwave8((k*allfreq)+thisphase), thiscutoff);    // qsub sets a minimum value called thiscutoff. If < thiscutoff, then bright = 0. Otherwise, bright = 128 (as defined in qsub)..
    leds[k] = CHSV(bgclr, 255, bgbright);                                     // First set a background colour, but fully saturated.
    leds[k] += ColorFromPalette( currentPalette, colorIndex, thisbright, currentBlending);    // Let's now add the foreground colour.
    colorIndex +=3;
  }
  
  bgclr++;
  
} // one_sine_pal()

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
    processButtonInputs();
    processClickWheelInputs();  

    EVERY_N_MILLISECONDS(thisdelay) {                           // FastLED based non-blocking delay to update/display the sequence.
        one_sine_pal(millis()>>4);
    }

    EVERY_N_MILLISECONDS(100) {
        uint8_t maxChanges = 24; 
        nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);   // AWESOME palette blending capability.
    }

    EVERY_N_SECONDS(5) {                                        // Change the target palette to a random one every 5 seconds.
        static uint8_t baseC = random8();                         // You can use this as a baseline colour if you want similar hues in the next line.
        targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
    }
    
    FastLED.show();
}
