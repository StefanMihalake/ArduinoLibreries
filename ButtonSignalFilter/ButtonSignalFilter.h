#ifndef ButtonSignalFilter_h
#define ButtonSignalFilter_h
#include "Arduino.h"

class ButtonSignalFilter{
  public:
    bool state;
    bool trigUp;
    bool trigDown;

    ButtonSignalFilter(int pin);
    bool Loop();

  private:
    unsigned long tIn;
    void State(int pin);
    int _pin;
    bool _lastStatePin;
    bool _lastState;
    bool _buttonState = false;
    unsigned long _debounceDelay = 5;
};
#endif
