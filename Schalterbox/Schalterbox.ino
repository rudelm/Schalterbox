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

boolean buzzerEnabled = false;
boolean redPressed = false;
boolean greenPressed = false;
boolean bluePressed = false;
boolean forwardPressed = false;
boolean backwardPressed = false;
boolean togglePride = false;

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

    // reset power saving on hardware change
    minutes = 0;
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

void rainbowAnimation(int hue, int speed) {
    int startHue = millis() * (128-hue)/255;

    byte hueChange = speed / NUM_LEDS;

    fill_rainbow(leds, NUM_LEDS, startHue, hueChange);
}

void sleepToBatterySave() {
    if (millis() - lastTime > 60000) {
        minutes ++;
        lastTime = millis();
    }

    if (minutes >= sleepTime) {
        max_bright = 0;
    }
}

// This function draws rainbows with an ever-changing,
// widely-varying set of parameters.
// taken from https://gist.github.com/kriegsman/964de772d64c502760e5
void pride() 
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;
 
  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);
  
  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;
  
  for( uint16_t i = 0 ; i < NUM_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);
    
    CRGB newcolor = CHSV( hue8, sat8, bri8);
    
    uint16_t pixelnumber = i;
    pixelnumber = (NUM_LEDS-1) - pixelnumber;
    
    nblend( leds[pixelnumber], newcolor, 64);
  }
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
                redValue += increment;
            } else {
                redValue = 255;
            }
        }
        if (greenPressed) {
            if ((greenValue >= 0) && (greenValue < 255)) {
                greenValue += increment;
            } else {
                greenValue = 255;
            }
        }
        if (bluePressed) {
            if ((blueValue >= 0) && (blueValue < 255)) {
                blueValue += increment;
            } else {
                blueValue = 255;
            }
        }

        if (!redPressed && !greenPressed && !bluePressed) {
            if ((max_bright > 0) && (max_bright < (20 - increment))) {
                max_bright += increment;
            } else {
                max_bright = 20;
            }
        }
    }
    else if (result == DIR_CCW) {
        Serial.println("CounterClockWise");
        playBeep();
        if (redPressed) {
            if ((redValue > 0) && (redValue <= 255)) {
                redValue -= increment;
            } else {
                redValue = 0;
            }
        }
        if (greenPressed) {
            if ((greenValue > 0) && (greenValue <= 255)) {
                greenValue -= increment;
            } else {
                greenValue = 0;
            }
        }
        if (bluePressed) {
            if ((blueValue > 0) && (blueValue <= 255)) {
                blueValue -= increment;
            } else {
                blueValue = 0;
            }
        }

        if (!redPressed && !greenPressed && !bluePressed) {
            if ((max_bright > increment) && (max_bright <= 20)) {
                max_bright -= increment;
            } else {
                max_bright = 1;
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

    FastLED.setBrightness(max_bright);

    if (forwardPressed) {
        if (bluePressed) {
            togglePride = !togglePride;
        }

        if (togglePride) {
            pride();
        } else {
            rainbowAnimation(redValue, greenValue);
        }
        //setColorRgb(redValue, greenValue, blueValue);
    }
    else if (backwardPressed) {
        setColorHsv(redValue, greenValue, blueValue);
    }
    printValues(redValue, greenValue, blueValue);

    FastLED.show();

    sleepToBatterySave();
}
