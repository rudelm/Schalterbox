/*
    Rotary Encoder - Interrupt Example
    
    The circuit:
    * encoder pin A to Arduino pin 3
    * encoder pin B to Arduino pin 4
    * encoder ground pin to ground (GND)
*/

#include <Rotary.h>

#define LED_PIN       13 // Its the internal LED on the Arduino

Rotary r = Rotary(3, 4);
int internalLedState = HIGH;

void setup() {
  pinMode(LED_PIN,OUTPUT); // Setup the LED
  digitalWrite(LED_PIN,internalLedState);
  Serial.begin(9600);
  cli();
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19) | (1 << PCINT20);
  sei();
  r.begin(true);
}

void loop() {
  digitalWrite(LED_PIN, internalLedState);
}

ISR(PCINT2_vect) {
  bool needToToggleLed = false;
  // if a LED toggle has been flagged :
    if (needToToggleLed) {
        // Toggle LED state :
        internalLedState = !internalLedState;
    }
  
  Serial.println("Interrupt");
  unsigned char result = r.process();
  if (result == DIR_NONE) {
    // do nothing
    Serial.println("NOP");
  }
  else if (result == DIR_CW) {
    Serial.println("ClockWise");
  }
  else if (result == DIR_CCW) {
    Serial.println("CounterClockWise");
  }
}
