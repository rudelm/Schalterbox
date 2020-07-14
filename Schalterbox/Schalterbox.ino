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
CRGB leds[NUM_LEDS];

uint8_t max_bright = 20;                                     // Overall brightness definition. It can be changed on the fly.

char logMessage[100];

long lastTime = 0;
long minutes = 0;
long sleepTime = 5;

int activeLedNumber = 0;
int ledInc = 1;
int ledSpeed = 1000/60;

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

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

boolean buzzerEnabled = false;

Rotary r = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);
Bounce * buttons = new Bounce[NUM_BUTTONS];

// declaring prototypes
void playMelody();
void playBeep();

void setup() {
    Serial.begin(115200);

    for (int i = 0; i < NUM_BUTTONS; i++) {
        buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP  );       //setup the bounce instance for the current button
        buttons[i].interval(25);                                   // interval in ms
    }

    buttons[BUZZER_SWITCH_ARRAY_POSITION].update();
    int buzzerState = buttons[BUZZER_SWITCH_ARRAY_POSITION].read();
    if ( buzzerState == LOW ) {
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

  sleepToBatterySave();
  FastLED.setBrightness(max_bright);
}


// Button, Switches, Clickwheel handling
void processButtonInputs() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    // Update the Bounce instance :
    buttons[i].update();
    if ( buttons[i].fell() ) {
      switch (i) {
        case ENCODER_BUTTON_ARRAY_POSITION:
            animationMode += 1;
            if (animationMode > 3) {
              animationMode = 0;
            }
            break;
        case RED_SWITCH_ARRAY_POSITION:
            redSelected = !redSelected;
            break;
        case GREEN_SWITCH_ARRAY_POSITION:
            greenSelected = !greenSelected;
            break;
        case BLUE_SWITCH_ARRAY_POSITION:
            blueSelected = !blueSelected;
            break;
        case FORWARD_SWITCH_ARRAY_POSITION:
          //animate0 requires these settings
          if (redSelected) {
            redDir = redDir * -1.0;
          }
          if (greenSelected) {
            greenDir = greenDir * -1.0;
          }
          if (blueSelected) {
            blueDir = blueDir * -1.0;
          }
          break;
        case BACKWARD_SWITCH_ARRAY_POSITION:
          //animate0 requires these settings
          if (redSelected) {
            redDir = redDir * -1.0;
          }
          if (greenSelected) {
            greenDir = greenDir * -1.0;
          }
          if (blueSelected) {
            blueDir = blueDir * -1.0;
          }
            break;
        case BUZZER_SWITCH_ARRAY_POSITION:
            Serial.println("Buzzer was enabled.");
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

    int currentButtonRead = buttons[i].read();
    if ( currentButtonRead == HIGH )
    {
      // buttons are not pressed
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
    // animate0 requires these settings
    if(redSelected) {
      redSpeed += 0.1;
    }
    if(greenSelected) {
      greenSpeed += 0.1;
    }
    if(blueSelected) {
      blueSpeed += 0.1;
    }

    // animate 2 and 3 require this setting
    if(!redSelected && !greenSelected && !blueSelected)
    {
      redSpeed += 0.1;
    }

    playBeep();
  }
  else if (result == DIR_CCW) {
    Serial.println("CounterClockWise");
    // animate0 requires these settings
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

    // animate 2 and 3 require this setting
    if(!redSelected && !greenSelected && !blueSelected)
    {
      redSpeed -= 0.1;
      if (redSpeed <= 0.1) {
          redSpeed = 0.1;
      }
    }
    
    playBeep();
  }
}

// Piezo speaker
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

  // reset power saving on hardware change
  minutes = 0;
}

// Battery management
void sleepToBatterySave() {
  if (millis() - lastTime > 60000) {
    minutes ++;
    lastTime = millis();
  }

  if (minutes >= sleepTime) {
    max_bright = 0;
  }
}

// Animation code from Sebastian
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

  processButtonInputs();
  processClickWheelInputs();  

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

  processButtonInputs();
  processClickWheelInputs();  

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

  processButtonInputs();
  processClickWheelInputs();  
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

  processButtonInputs();
  processClickWheelInputs();  
}
