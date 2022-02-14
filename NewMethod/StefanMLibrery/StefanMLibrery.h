#ifndef StefanMLibrery_h
#define StefanMLibrery_h
#include "Arduino.h"
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>

class ButtonSignalFilter{
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



//===========================================================================




class Dimmer{
  private:
    ButtonSignalFilter bfilter;
    // I/O PIN
    
                       // the number of the LED pin
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
    float count = 255;
    float newCount = 0;
    bool maxReach = false;
    bool dim = 0;
    // LOOP LIFE VARIABLE------------------
    unsigned long startLoop = 0;
    unsigned long loopInterval = 10;
    unsigned long t1 = 0;

    
  public:
  int _ledPin;  
    Dimmer(int id, int bPin, int ledPin, int minVal, int maxVal, unsigned long dimTemp, unsigned long dimDelay);
    void Start();
    void ReadSerial(String comand, String dim="");
    void SetMinMax(int min, int max);
    void ReadModBus(int on, int off);
    void SetDim(int dim);
    int _id;
};

//======================================================================================


class Orchestrator {
  private:
  String first;
  String second;
  String third;
  int  _startCoil; 
  int _startReg;
    void SplitString(String str, char sep);
  public:
    Orchestrator(int idDispositivo, int comPort, int itemNumber, int  startCoil, int startReg);
    void Start(Dimmer dimmers[]);
};


#endif