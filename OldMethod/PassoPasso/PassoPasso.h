#ifndef PassoPasso_h
#define PassoPasso_h
#include "Arduino.h"

class PassoPasso{
  public:

    PassoPasso(int pin, int ledPin);
    void Start();

  private:
    // Variables will change:
    int ledState = HIGH;                      // the current state of the output pin
    bool buttonState = false;                 // the current reading from the input pin
    bool lastButtonState;                     // the previous reading from the input pin
    bool buttonPress;                         // actual button pressing
    int _buttonPin;                                 // pin of the button
    int _ledPin;
    // Variable used to cleen the input signal
    unsigned long lastDebounceTime = 0;       // time pass from the press
    unsigned long debounceDelay = 5;          // millisecondo pass from the press
    void State(int _buttonPin, int ledPin);

};
#endif
