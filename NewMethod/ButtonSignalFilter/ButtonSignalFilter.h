#ifndef ButtonSignalFilter_h
#define ButtonSignalFilter_h
#include "Arduino.h"

class ButtonSignalFilter
{
private:
  unsigned long tIn;
  int _pin;
  bool _lastStatePin;
  bool _lastState;
  bool _buttonState = false;
  unsigned long _debounceDelay = 5;

public:
  bool state;
  bool trigUp;
  bool trigDown;
  void State(int pin);
  ButtonSignalFilter();
  ButtonSignalFilter(int pin);
  bool Loop();
};
#endif
