#ifndef Dimmer_h
#define Dimmer_h
#include "Arduino.h"

class Dimmer{
  public:

    Dimmer(int ledPin, int minVal, int maxVal, unsigned long dimTemp,  unsigned long dimDelay);
    void Start(bool trigUp, bool trigDown, bool state);
    void ReadSerial(String comand);

  private:
    // I/O PIN
    int _ledPin;                     // the number of the LED pin
    // LED VARIABLE-----------------------
    float ledState = 0;                       // the current state of the output pin
    float lastLedState = 255;
    // BUTTON VARIABLE
    bool buttonState = false;                  // the current reading from the input pin
    bool lastButtonState;                      // the previous reading from the input pin
    bool buttonPress;
    // DIM VARIALBE------------------------
    unsigned long _dimDelay ;
    unsigned long _dimTemp;
    unsigned long tDim = 0;
    unsigned long _max;
    unsigned long _min;
    int distance;
    float onePCent;
    // DELAY DIM VARIABLE------------------
    float count = 0;
    float newCount = 0;
    bool maxReach = false;
    bool dim = 0;
    // LOOP LIFE VARIABLE------------------
    unsigned long startLoop = 0;
    unsigned long loopInterval = 10;
    unsigned long t1 = 0;
};
#endif