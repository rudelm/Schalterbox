#include <FastLED.h>
#include <Rotary.h>
#include <Bounce2.h>
#include "pitches.h"

#define ENCODER_PIN_BUTTON 2
#define ENCODER_PIN_A 3
#define ENCODER_PIN_B 4
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
#define COLOR_ORDER RGB
CRGB leds[NUM_LEDS];

#define BRIGHTNESS  128

int internalLedState = LOW;
int activeLedNumber = 0;
int ledInc = 1;
int ledSpeed = 1000/60;

byte animationMode = 0;

byte redFactor = 1;
byte greenFactor = 1;
byte blueFactor = 1;

double redAmount = 255.0;
double greenAmount = 255.0;
double blueAmount = 255.0;

double redDeg = 0.0;
double greenDeg = 0.0;
double blueDeg = 0.0;

double redSpeed = 0.1;
double greenSpeed = 0.2;
double blueSpeed = 0.3;

double redDir = 1.0;
double greenDir = 1.0;
double blueDir = 1.0;

bool redSelected = true;
bool greenSelected = false;
bool blueSelected = false;

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
  r.begin(true);

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

void playMelody() {
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

void playBeep() {
    tone(BUZZER_PIN, 1000, 100);
}

void updateButtons() {
  for (int i = 0; i < NUM_BUTTONS; i++)  {
    buttons[i].update();
  }
}

void checkForAnimationToggle() {
  // ENCODER_PIN_BUTTON
  if (buttons[0].fell()) {
    animationMode += 1;
    if (animationMode > 3) {
      animationMode = 0;
    }
  }
}

void checkForRGBDips() {
  // BUTTON_RED_PIN
  if (buttons[1].fell()) {
    redSelected = !redSelected;
  }
  // BUTTON_GREEN_PIN
  if (buttons[2].fell()) {
    greenSelected = !greenSelected;
  }
  // BUTTON_BLUE_PIN
  if (buttons[3].fell()) {
    blueSelected = !blueSelected;
  }
}


void animate0() {
  FastLED.clear();

  for (int i = 0; i < NUM_LEDS; i++) {
    byte redValue = byte(abs(sin( (redDeg + i) / (PI * 2) ) * 255));
    byte greenValue = byte(abs(sin( (greenDeg + i) / (PI * 2) ) * 255));
    byte blueValue = byte(abs(sin( (blueDeg + i) / (PI * 2) ) * 255));

    if(!redSelected) {
      redValue = 0.0;
    }
    if(!greenSelected) {
      greenValue = 0.0;
    }
    if(!blueSelected) {
      blueValue = 0.0;
    }

    leds[i] = CRGB(redValue, greenValue, blueValue);
  }

  FastLED.show();
  FastLED.delay(ledSpeed);

  updateButtons();
  checkForAnimationToggle();
  checkForRGBDips();

  // FORWARD_SWITCH_PIN
  if (buttons[4].fell()) {
    if (redSelected) {
      redDir = redDir * -1.0;
    }
    if (greenSelected) {
      greenDir = greenDir * -1.0;
    }
    if (blueSelected) {
      blueDir = blueDir * -1.0;
    }
  }
  if (buttons[5].fell()) {
    if (redSelected) {
      redDir = redDir * -1.0;
    }
    if (greenSelected) {
      greenDir = greenDir * -1.0;
    }
    if (blueSelected) {
      blueDir = blueDir * -1.0;
    }
  }

  unsigned char result = r.process();
  if (result == DIR_NONE) {
      // do nothing
  } else if (result == DIR_CW) {
    Serial.println("ClockWise");
    if(redSelected) {
      redSpeed += 0.1;
    }
    if(greenSelected) {
      greenSpeed += 0.1;
    }
    if(blueSelected) {
      blueSpeed += 0.1;
    }
  } else if (result == DIR_CCW) {
    Serial.println("CounterClockWise");
    if(redSelected) {
      redSpeed -= 0.1;
    }
    if(greenSelected) {
      greenSpeed -= 0.1;
    }
    if(blueSelected) {
      blueSpeed -= 0.1;
    }

    if (redSpeed <= 0.1) {
        redSpeed = 0.1;
    }
    if (greenSpeed <= 0.1) {
        greenSpeed = 0.1;
    }
    if (blueSpeed <= 0.1) {
        blueSpeed = 0.1;
    }
  }

  redDeg += redSpeed * redDir;
  greenDeg += greenSpeed * greenDir;
  blueDeg += blueSpeed * blueDir;
  activeLedNumber = (activeLedNumber + ledInc) % NUM_LEDS;
}

void animate1() {
  FastLED.clear();

  byte redValue = byte(redAmount);
  byte greenValue = byte(greenAmount);
  byte blueValue = byte(blueAmount);

  if(!redSelected) {
    redValue = 0.0;
  }
  if(!greenSelected) {
    greenValue = 0.0;
  }
  if(!blueSelected) {
    blueValue = 0.0;
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(redValue, greenValue, blueValue);
  }

  FastLED.show();
  FastLED.delay(ledSpeed);

  updateButtons();
  checkForAnimationToggle();
  checkForRGBDips();

  unsigned char result = r.process();
  if (result == DIR_NONE) {
      // do nothing
  } else if (result == DIR_CW) {
    Serial.println("ClockWise");
    if(redSelected) {
      redAmount += 16.0;
    }
    if(greenSelected) {
      greenAmount += 16.0;
    }
    if(blueSelected) {
      blueAmount += 16.0;
    }

    if (redAmount > 255.0) {
      redAmount = 255.0;
    }
    if (greenAmount > 255.0) {
      greenAmount = 255.0;
    }
    if (blueAmount > 255.0) {
      blueAmount = 255.0;
    }
  } else if (result == DIR_CCW) {
    Serial.println("CounterClockWise");
    if(redSelected) {
      redAmount -= 16.0;
    }
    if(greenSelected) {
      greenAmount -= 16.0;
    }
    if(blueSelected) {
      blueAmount -= 16.0;
    }

    if (redAmount < 0.0) {
        redAmount = 0.0;
    }
    if (greenAmount < 0.0) {
        greenAmount = 0.0;
    }
    if (blueAmount < 0.0) {
        blueAmount = 0.0;
    }
  }
}

void animate2() {
  FastLED.clear();


  for (int i = 0; i < NUM_LEDS; i++) {
    byte redValue = byte(redAmount);

    leds[i] = CHSV(redValue, 255, 255);
  }

  redAmount = redAmount + redSpeed;
  if (redAmount > 255) {
    redAmount = 0;
  }

  FastLED.show();
  FastLED.delay(ledSpeed);

  updateButtons();
  checkForAnimationToggle();
  checkForRGBDips();

  unsigned char result = r.process();
  if (result == DIR_NONE) {
      // do nothing
  } else if (result == DIR_CW) {
    redSpeed += 0.1;
  } else if (result == DIR_CCW) {
    redSpeed -= 0.1;
    if (redSpeed <= 0.1) {
        redSpeed = 0.1;
    }
  }
}

void animate3() {
  FastLED.clear();


  for (int i = 0; i < NUM_LEDS; i++) {
    byte redValue = byte(redAmount + i*256 / NUM_LEDS);

    leds[i] = CHSV(redValue, 255, 255);
  }

  redAmount = redAmount + redSpeed;
  if (redAmount > 255) {
    redAmount = 0;
  }

  FastLED.show();
  FastLED.delay(ledSpeed);

  updateButtons();
  checkForAnimationToggle();
  checkForRGBDips();

  unsigned char result = r.process();
  if (result == DIR_NONE) {
      // do nothing
  } else if (result == DIR_CW) {
    redSpeed += 0.1;
  } else if (result == DIR_CCW) {
    redSpeed -= 0.1;
    if (redSpeed <= 0.1) {
        redSpeed = 0.1;
    }
  }
}

void loop()
{
  if (animationMode == 0) {
    animate0();
  }
  if (animationMode == 1) {
    animate1();
  }
  if (animationMode == 2) {
    animate2();
  }
  if (animationMode == 3) {
    animate3();
  }
}
