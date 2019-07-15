#include <FastLED.h>
#include <Rotary.h>
#include <Bounce2.h>
#include "pitches.h"

#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
#define ENCODER_PIN_BUTTON 4
#define BUZZER_PIN 5
#define DATA_PIN      11
#define CLOCK_PIN     12
#define LED_PIN       13 // Its the internal LED on the Arduino
#define BUTTON_RED_PIN 22
#define BUTTON_GREEN_PIN 24
#define BUTTON_BLUE_PIN 26
#define FORWARD_SWITCH_PIN 28
#define BACKWARD_SWITCH_PIN 30
#define ROTATION_SWITCH_POS_1_PIN 32
#define ROTATION_SWITCH_POS_2_PIN 34
#define ROTATION_SWITCH_POS_3_PIN 36
#define ROTATION_SWITCH_POS_4_PIN 38
#define ROTATION_SWITCH_POS_5_PIN 40
#define ROTATION_SWITCH_POS_6_PIN 42



#define NUM_BUTTONS 12
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {ENCODER_PIN_BUTTON, BUTTON_RED_PIN, BUTTON_GREEN_PIN, BUTTON_BLUE_PIN,
 FORWARD_SWITCH_PIN, BACKWARD_SWITCH_PIN, ROTATION_SWITCH_POS_1_PIN, ROTATION_SWITCH_POS_2_PIN, ROTATION_SWITCH_POS_3_PIN,
 ROTATION_SWITCH_POS_4_PIN, ROTATION_SWITCH_POS_5_PIN, ROTATION_SWITCH_POS_6_PIN};

// Information about the LED strip itself
#define NUM_LEDS    12
#define CHIPSET     WS2801
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define BRIGHTNESS  128


// FastLED v2.1 provides two color-management controls:
//   (1) color correction settings for each LED strip, and
//   (2) master control of the overall output 'color temperature' 
//
// THIS EXAMPLE demonstrates the second, "color temperature" control.
// It shows a simple rainbow animation first with one temperature profile,
// and a few seconds later, with a different temperature profile.
//
// The first pixel of the strip will show the color temperature.
//
// HELPFUL HINTS for "seeing" the effect in this demo:
// * Don't look directly at the LED pixels.  Shine the LEDs aganst
//   a white wall, table, or piece of paper, and look at the reflected light.
//
// * If you watch it for a bit, and then walk away, and then come back 
//   to it, you'll probably be able to "see" whether it's currently using
//   the 'redder' or the 'bluer' temperature profile, even not counting
//   the lowest 'indicator' pixel.
//
//
// FastLED provides these pre-conigured incandescent color profiles:
//     Candle, Tungsten40W, Tungsten100W, Halogen, CarbonArc,
//     HighNoonSun, DirectSunlight, OvercastSky, ClearBlueSky,
// FastLED provides these pre-configured gaseous-light color profiles:
//     WarmFluorescent, StandardFluorescent, CoolWhiteFluorescent,
//     FullSpectrumFluorescent, GrowLightFluorescent, BlackLightFluorescent,
//     MercuryVapor, SodiumVapor, MetalHalide, HighPressureSodium,
// FastLED also provides an "Uncorrected temperature" profile
//    UncorrectedTemperature;

#define TEMPERATURE_1 Tungsten100W
#define TEMPERATURE_2 OvercastSky

// How many seconds to show each temperature before switching
#define DISPLAYTIME 20
// How many seconds to show black between switches
#define BLACKTIME   3

int internalLedState = LOW;

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

Rotary r = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);
Bounce * buttons = new Bounce[NUM_BUTTONS];

// declaring prototypes
void playMelody();
void playBeep();

void setup() {
    Serial.begin(9600);
    r.begin();
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
    sei();

    for (int i = 0; i < NUM_BUTTONS; i++) {
        buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP  );       //setup the bounce instance for the current button
        buttons[i].interval(25);              // interval in ms
    }

    
    pinMode(LED_PIN,OUTPUT); // Setup the LED
    digitalWrite(LED_PIN,internalLedState);
    playMelody();

    delay(3000); // power-up safety delay
    // It's important to set the color correction for your LED strip here,
    // so that colors can be more accurately rendered through the 'temperature' profiles
    FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
    FastLED.setBrightness( BRIGHTNESS );
}

// Interrupt service routine to react on rotary encoder
ISR(PCINT2_vect) {
    unsigned char result = r.process();
    if (result == DIR_NONE) {
        // do nothing
    }
    else if (result == DIR_CW) {
        Serial.println("ClockWise");
    }
    else if (result == DIR_CCW) {
        Serial.println("CounterClockWise");
    }
}

void playMelody() {
    // iterate over the notes of the melody:    
    for (int thisNote = 0; thisNote < 8; thisNote++) {

        // to calculate the note duration, take one second divided by the note type.
        //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
        int noteDuration = 1000 / noteDurations[thisNote];
        tone(5, melody[thisNote], noteDuration);

        // to distinguish the notes, set a minimum time between them.
        // the note's duration + 30% seems to work well:
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        // stop the tone playing:
        noTone(5);
    }
}

void playBeep() {

}

void loop()
{
    bool needToToggleLed = false;

    for (int i = 0; i < NUM_BUTTONS; i++)  {
        // Update the Bounce instance :
        buttons[i].update();
        // If it fell, flag the need to toggle the LED
        if ( buttons[i].fell() ) {
            needToToggleLed = true;
        }
    }

    // if a LED toggle has been flagged :
    if ( needToToggleLed ) {
        // Toggle LED state :
        internalLedState = !internalLedState;
        digitalWrite(LED_PIN, internalLedState);
    }

    // draw a generic, no-name rainbow
    static uint8_t starthue = 0;
    fill_rainbow( leds + 5, NUM_LEDS - 5, --starthue, 20);

    // Choose which 'color temperature' profile to enable.
    uint8_t secs = (millis() / 1000) % (DISPLAYTIME * 2);
    if( secs < DISPLAYTIME) {
        FastLED.setTemperature( TEMPERATURE_1 ); // first temperature
        leds[0] = TEMPERATURE_1; // show indicator pixel
    } else {
        FastLED.setTemperature( TEMPERATURE_2 ); // second temperature
        leds[0] = TEMPERATURE_2; // show indicator pixel
    }

    // Black out the LEDs for a few secnds between color changes
    // to let the eyes and brains adjust
    if( (secs % DISPLAYTIME) < BLACKTIME) {
        memset8( leds, 0, NUM_LEDS * sizeof(CRGB));
    }
    
    FastLED.show();
    FastLED.delay(8);
}
