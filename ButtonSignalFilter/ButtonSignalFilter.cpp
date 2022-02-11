#include "Arduino.h"
#include "ButtonSignalFilter.h"

ButtonSignalFilter::ButtonSignalFilter(int pin) {
  ButtonSignalFilter::State(pin);

}

void ButtonSignalFilter::State(int pin) {
  pinMode(pin, INPUT);
  _pin = pin;
}

bool ButtonSignalFilter::Loop() {
  _buttonState = digitalRead(_pin);

  if (_buttonState != _lastState) {
    tIn = millis();
  } else {
    if ((millis() - tIn) > _debounceDelay) {
      if (_buttonState != _lastStatePin) {

        if (_buttonState == LOW) {
          trigUp = false;
          trigDown = true;
        }
        else {
          trigUp = true;
          trigDown = false;
        }

      }
      else {
        trigUp = false;
        trigDown = false;
      }
      state = _buttonState;
      _lastStatePin = _buttonState;
    }
  }
  _lastState = _buttonState;
  return state;
}
